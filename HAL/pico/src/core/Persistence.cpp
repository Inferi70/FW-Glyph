/*
 * This file is part of HayBox
 * Copyright (C) 2024 Jonathan Haylett
 *
 * HayBox is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software. If not, see <http://www.gnu.org/licenses/>.
 */

#include "core/Persistence.hpp"

#include "stdlib.hpp"

#include <CRC32.h>
#include <LittleFS.h>
#include <cstring>
#include <memory>
#include <pb_arduino.h>
#include <pb_decode.h>
#include <pb_encode.h>

Persistence::Persistence() {
    LittleFS.begin();
}

Persistence::~Persistence() {
    LittleFS.end();
}

const char *Persistence::LastError() const {
    return _last_error;
}

bool Persistence::SetError(const char *message) {
    strncpy(_last_error, message ? message : "unknown", sizeof(_last_error) - 1);
    _last_error[sizeof(_last_error) - 1] = '\0';
    return false;
}

bool Persistence::SaveConfig(Config &config) {
    _last_error[0] = '\0';

    // Make sure config encodes correctly.
    size_t encoded_size;
    if (!pb_get_encoded_size(&encoded_size, Config_fields, &config)) {
        return SetError("pb_get_encoded_size failed");
    }

    auto encoded = std::make_unique<uint8_t[]>(encoded_size);
    if (!encoded) {
        return SetError("buffer allocation failed");
    }

    // Encode Protobuf data in RAM first so the flash write is a single validated pass.
    pb_ostream_t ostream = pb_ostream_from_buffer(encoded.get(), encoded_size);
    if (!pb_encode(&ostream, Config_fields, &config)) {
        return SetError("pb_encode failed");
    }

    CRC32 crc;
    for (size_t i = 0; i < ostream.bytes_written; i++) {
        crc.update(encoded[i]);
    }

    ConfigHeader header = {
        .config_size = ostream.bytes_written,
        .config_crc = crc.finalize(),
    };

    File config_file = LittleFS.open(config_filename, "w+");
    if (!config_file) {
        return SetError("LittleFS.open(w+) failed");
    }

    if (config_file.write((uint8_t *)&header, sizeof(ConfigHeader)) != sizeof(ConfigHeader)) {
        config_file.close();
        return SetError("header write failed");
    }
    if (config_file.write(encoded.get(), ostream.bytes_written) != ostream.bytes_written) {
        config_file.close();
        return SetError("config write failed");
    }
    config_file.flush();

    // Persist changes.
    config_file.close();

    if (!CheckSavedConfig()) {
        if (_last_error[0] == '\0') {
            SetError("post-save validation failed");
        }
        return false;
    }

    return true;
}

bool Persistence::LoadConfig(Config &config) {
    _last_error[0] = '\0';

    // Open file to load config data from.
    File config_file = LittleFS.open(config_filename, "r");
    if (!config_file) {
        return SetError("LittleFS.open(r) failed");
    }

    if (!CheckSavedConfig(config_file)) {
        config_file.close();
        return false;
    }

    // Seek to start of Protobuf data.
    if (!config_file.seek(config_offset)) {
        config_file.close();
        return SetError("seek to config body failed");
    }

    // Reset config defaults first, so config is completely replaced rather than merged with
    // defaults.
    config = Config_init_default;

    // Decode streamed Protobuf data into config struct.
    pb_istream_t istream = as_pb_istream(config_file, (size_t)config_file.available());
    if (!pb_decode(&istream, Config_fields, &config)) {
        config_file.close();
        return SetError("pb_decode failed");
    }

    config_file.close();
    return true;
}

bool Persistence::CheckSavedConfig() {
    _last_error[0] = '\0';

    // Open file to load config data from.
    File config_file = LittleFS.open(config_filename, "r");
    if (!config_file) {
        return SetError("LittleFS.open(r) failed");
    }

    bool is_valid = CheckSavedConfig(config_file);
    config_file.close();
    return is_valid;
}

size_t Persistence::LoadConfigRaw(Print &out, bool validate) {
    _last_error[0] = '\0';

    // Open file to load config data from.
    File config_file = LittleFS.open(config_filename, "r");
    if (!config_file) {
        SetError("LittleFS.open(r) failed");
        return false;
    }

    // Optionally perform validation.
    if (validate && !CheckSavedConfig(config_file)) {
        config_file.close();
        return false;
    }

    // Seek to start of Protobuf data.
    if (!config_file.seek(config_offset)) {
        config_file.close();
        SetError("seek to config body failed");
        return false;
    }

    // Write raw Protobuf encoded data to output stream.
    int value;
    while ((value = config_file.read()) != -1) {
        out.write((uint8_t)value);
    }

    config_file.close();
    return true;
}

bool Persistence::CheckSavedConfig(File &config_file) {
    size_t file_size = config_file.size();
    if (file_size < config_offset) {
        return SetError("config file too small");
    }

    // Read file header.
    ConfigHeader header;
    size_t bytes_read = config_file.read((uint8_t *)&header, sizeof(ConfigHeader));
    if (bytes_read < sizeof(ConfigHeader)) {
        return SetError("header read failed");
    }

    // Validate config length.
    size_t config_size = file_size - config_offset;
    if (config_size != header.config_size) {
        return SetError("config size mismatch");
    }

    // Calculate CRC for file contents and compare with CRC in header.
    CRC32 crc;
    int value;
    while ((value = config_file.read()) != -1) {
        crc.update((uint8_t)value);
    }
    if (crc.finalize() != header.config_crc) {
        return SetError("config CRC mismatch");
    }

    return true;
}

Persistence persistence;
