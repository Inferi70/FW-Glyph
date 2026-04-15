#include "modes/64.hpp"

#define ANALOG_STICK_MIN 28
#define ANALOG_STICK_NEUTRAL 128
#define ANALOG_STICK_MAX 228

Smash64::Smash64() : ControllerMode() {
}

void Smash64::SetConfig(GameModeConfig &config) {
    InputMode::SetConfig(config);
}

void Smash64::HandleSocd(InputState &inputs) {
    InputMode::HandleSocd(inputs);
}

void Smash64::UpdateDigitalOutputs(const InputState &inputs, OutputState &outputs) {
    outputs.a = inputs.rt1;
    outputs.b = inputs.rf1;
    outputs.triggerRDigital = inputs.rf5; //inputs.rf3; //R

    outputs.buttonR = inputs.rf3; //inputs.rf5; //Z

    outputs.start = inputs.mb7;
    outputs.select = inputs.mb6;
    outputs.home = inputs.mb5;
    outputs.capture = inputs.mb4;

    outputs.triggerLDigital = inputs.lf4; //inputs.rf9; //L

    outputs.leftStickLeft = inputs.lf3;
    outputs.leftStickRight = inputs.lf1;
    outputs.leftStickDown = inputs.lf2;
    outputs.leftStickUp = inputs.rf4;

    outputs.rightStickLeft = inputs.rf7;
    outputs.rightStickRight = inputs.rf8;
    outputs.rightStickDown = inputs.rf2;
    outputs.rightStickUp = inputs.rf6;

    outputs.dpadUp = inputs.lt6;
    outputs.dpadDown = inputs.lf7;
    outputs.dpadLeft = inputs.lf8;
    outputs.dpadRight = inputs.lf6;
}

void Smash64::UpdateAnalogOutputs(const InputState &inputs, OutputState &outputs, CommunicationBackendId backend_id) {
    OutputState outputBuffer = OutputState();

    // Coordinate calculations to make modifier handling simpler.
    UpdateDirections(
        inputs.lf3, // Left
        inputs.lf1, // Right
        inputs.lf2, // Down
        inputs.rf4, // Up
        false, // C-Left
        false, // C-Right
        false, // C-Down
        false, // C-Up
        ANALOG_STICK_MIN,
        ANALOG_STICK_NEUTRAL,
        ANALOG_STICK_MAX,
        outputBuffer
    );

    /* inputs.lt1 = mx | inputs.lt2 = my */

    outputs.leftStickX = outputBuffer.leftStickX;
    outputs.leftStickY = outputBuffer.leftStickY;
    outputs.rightStickX = outputBuffer.rightStickX;
    outputs.rightStickY = outputBuffer.rightStickY;
    outputs.triggerLAnalog = outputBuffer.triggerLAnalog;
    outputs.triggerRAnalog = outputBuffer.triggerRAnalog;
}
