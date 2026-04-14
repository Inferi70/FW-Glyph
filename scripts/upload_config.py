#!/usr/bin/env python3
import argparse
import json
import re
import shutil
import subprocess
import sys
import tempfile
import time
from pathlib import Path

import serial


CMD_GET_DEVICE_INFO = 1
CMD_SET_DEVICE_INFO = 2
CMD_GET_CONFIG = 3
CMD_SET_CONFIG = 4
CMD_ERROR = 5
CMD_SUCCESS = 6


ENUM_PREFIXES = (
    "BTN_",
    "SOCD_",
    "MODE_",
    "COMMS_BACKEND_",
    "LAYOUT_",
    "OUT_",
    "RGB_ANIM_",
    "DASHBOARD_",
    "HID_KEY_",
    "GP_",
)


def cobs_encode(data: bytes) -> bytes:
    out = bytearray()
    code_index = 0
    out.append(0)
    code = 1

    for byte in data:
        if byte == 0:
            out[code_index] = code
            code_index = len(out)
            out.append(0)
            code = 1
        else:
            out.append(byte)
            code += 1
            if code == 0xFF:
                out[code_index] = code
                code_index = len(out)
                out.append(0)
                code = 1

    out[code_index] = code
    out.append(0)
    return bytes(out)


def cobs_decode(data: bytes) -> bytes:
    out = bytearray()
    index = 0
    while index < len(data):
        code = data[index]
        if code == 0:
            raise ValueError("invalid COBS frame: zero byte in payload")
        index += 1
        end = index + code - 1
        if end > len(data):
            raise ValueError("invalid COBS frame: code overruns payload")
        out.extend(data[index:end])
        index = end
        if code != 0xFF and index < len(data):
            out.append(0)
    return bytes(out)


def load_json(path: str):
    if path == "-":
        return json.load(sys.stdin)
    with open(path, "r", encoding="utf-8") as handle:
        return json.load(handle)


def is_enum_token(value: str) -> bool:
    return value.startswith(ENUM_PREFIXES)


def camel_to_snake(name: str) -> str:
    return re.sub(r"(?<!^)(?=[A-Z])", "_", name).lower()


def scalar_to_textproto(value, indent: int) -> str:
    if isinstance(value, bool):
        return "true" if value else "false"
    if isinstance(value, (int, float)):
        return str(value)
    if isinstance(value, str):
        if is_enum_token(value):
            return value
        return json.dumps(value)
    raise TypeError(f"unsupported scalar type: {type(value).__name__}")


def json_to_textproto(obj, indent: int = 0) -> str:
    lines = []
    prefix = "  " * indent

    if not isinstance(obj, dict):
        raise TypeError("top-level config must be a JSON object")

    for key, value in obj.items():
        field_name = camel_to_snake(key)
        if value is None:
            continue
        if isinstance(value, list):
            for item in value:
                if isinstance(item, dict):
                    lines.append(f"{prefix}{field_name} {{")
                    nested = json_to_textproto(item, indent + 1)
                    if nested:
                        lines.append(nested)
                    lines.append(f"{prefix}}}")
                else:
                    lines.append(
                        f"{prefix}{field_name}: {scalar_to_textproto(item, indent)}"
                    )
        elif isinstance(value, dict):
            lines.append(f"{prefix}{field_name} {{")
            nested = json_to_textproto(value, indent + 1)
            if nested:
                lines.append(nested)
            lines.append(f"{prefix}}}")
        else:
            lines.append(f"{prefix}{field_name}: {scalar_to_textproto(value, indent)}")

    return "\n".join(lines)


def find_proto(project_root: Path) -> Path:
    candidates = [
        project_root / ".pio/libdeps/glyph_mk6/HayBox-proto/config.proto",
        project_root / ".pio/libdeps/glyph_mk6_usb_host/HayBox-proto/config.proto",
    ]
    for candidate in candidates:
        if candidate.exists():
            return candidate
    raise FileNotFoundError(
        "Could not find config.proto in .pio/libdeps. Run `pio run -e glyph_mk6` first."
    )


def encode_config(proto_path: Path, textproto: str) -> bytes:
    protoc = shutil.which("protoc")
    if not protoc:
        raise FileNotFoundError("`protoc` not found in PATH")

    with tempfile.TemporaryDirectory() as temp_dir:
        text_path = Path(temp_dir) / "config.textproto"
        bin_path = Path(temp_dir) / "config.bin"
        text_path.write_text(textproto, encoding="utf-8")

        result = subprocess.run(
            [
                protoc,
                f"--proto_path={proto_path.parent}",
                f"--encode=Config",
                str(proto_path),
            ],
            input=textproto.encode("utf-8"),
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            check=False,
        )
        if result.returncode != 0:
            raise RuntimeError(result.stderr.decode("utf-8", errors="replace").strip())

        bin_path.write_bytes(result.stdout)
        return bin_path.read_bytes()


def read_frame(port: serial.Serial, timeout_s: float) -> bytes:
    deadline = time.monotonic() + timeout_s
    frame = bytearray()

    while time.monotonic() < deadline:
        chunk = port.read(1)
        if not chunk:
            continue
        byte = chunk[0]
        if byte == 0:
            if frame:
                return bytes(frame)
            continue
        frame.append(byte)

    raise TimeoutError("timed out waiting for response frame")


def exchange_set_config(port: serial.Serial, payload: bytes, timeout_s: float) -> int:
    request = bytes([CMD_SET_CONFIG]) + payload
    port.write(cobs_encode(request))
    port.flush()

    while True:
        raw_frame = read_frame(port, timeout_s)
        frame = cobs_decode(raw_frame)
        if not frame:
            continue

        command = frame[0]
        body = frame[1:]

        if command == CMD_SUCCESS:
            print("Device returned CMD_SUCCESS; reboot should follow.")
            return 0
        if command == CMD_ERROR:
            message = body.decode("utf-8", errors="replace")
            print(f"Device returned CMD_ERROR: {message}", file=sys.stderr)
            return 1

        print(f"Ignoring unexpected response command {command}")


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Upload a FW-Glyph configurator JSON file over serial using the device protobuf."
    )
    parser.add_argument("json_path", help="Path to config JSON file, or - for stdin")
    parser.add_argument("--port", required=True, help="Serial device, e.g. /dev/ttyACM0")
    parser.add_argument("--baud", type=int, default=115200, help="Serial baud rate")
    parser.add_argument(
        "--timeout",
        type=float,
        default=5.0,
        help="Seconds to wait for a configurator response",
    )
    parser.add_argument(
        "--dump-textproto",
        action="store_true",
        help="Print the generated textproto before upload",
    )
    args = parser.parse_args()

    project_root = Path(__file__).resolve().parent.parent
    proto_path = find_proto(project_root)
    config_json = load_json(args.json_path)
    textproto = json_to_textproto(config_json)

    if args.dump_textproto:
        print(textproto)

    payload = encode_config(proto_path, textproto)
    print(f"Encoded config payload: {len(payload)} bytes")

    try:
        with serial.Serial(args.port, args.baud, timeout=0.25, write_timeout=2.0) as port:
            return exchange_set_config(port, payload, args.timeout)
    except serial.SerialException as exc:
        print(f"Serial error: {exc}", file=sys.stderr)
        return 1
    except (FileNotFoundError, RuntimeError, TimeoutError, ValueError, TypeError) as exc:
        print(str(exc), file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
