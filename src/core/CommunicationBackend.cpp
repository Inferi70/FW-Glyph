#include "core/CommunicationBackend.hpp"

#include "core/ControllerMode.hpp"
#include "core/InputSource.hpp"
#include "core/state.hpp"

#include <config.pb.h>

CommunicationBackend::CommunicationBackend(
    InputState &inputs,
    InputSource **input_sources,
    size_t input_source_count
)
    : _inputs(inputs),
      _input_source_manager(input_sources, input_source_count) {
    _gamemode = nullptr;
}

InputState &CommunicationBackend::GetInputs() {
    return _inputs;
}

OutputState &CommunicationBackend::GetOutputs() {
    return _outputs;
}

void CommunicationBackend::ScanInputs() {
    _input_source_manager.ScanInputs(_inputs);
}

void CommunicationBackend::ScanInputs(InputScanSpeed input_source_filter) {
    _input_source_manager.ScanInputs(_inputs, input_source_filter);
}

void CommunicationBackend::ResetOutputs() {
    _outputs = OutputState();
}

void CommunicationBackend::UpdateOutputs() {
    //ResetOutputs();
    if (_gamemode != nullptr) {
        _gamemode->UpdateOutputs(_inputs, _outputs, BackendId());
    }
}

CommunicationBackendId CommunicationBackend::BackendId() {
    return COMMS_BACKEND_UNSPECIFIED;
}

void CommunicationBackend::SetGameMode(InputMode *gamemode) {
    _gamemode = gamemode;
}

InputMode *CommunicationBackend::CurrentGameMode() {
    return _gamemode;
}
