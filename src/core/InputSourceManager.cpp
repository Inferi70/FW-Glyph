#include "core/InputSourceManager.hpp"

InputSourceManager::InputSourceManager(
    InputSource **input_sources,
    size_t input_source_count
)
    : _input_sources(input_sources),
      _input_source_count(input_source_count) {}

void InputSourceManager::ScanInputs(InputState &inputs) {
    for (size_t i = 0; i < _input_source_count; i++) {
        _input_sources[i]->UpdateInputs(inputs);
        inputs.counter++;
    }
}

void InputSourceManager::ScanInputs(
    InputState &inputs,
    InputScanSpeed input_source_filter
) {
    for (size_t i = 0; i < _input_source_count; i++) {
        InputSource *input_source = _input_sources[i];
        if (input_source->ScanSpeed() == input_source_filter) {
            input_source->UpdateInputs(inputs);
        }
        inputs.counter++;
    }
}
