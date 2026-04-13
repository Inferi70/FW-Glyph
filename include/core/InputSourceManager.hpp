#ifndef _CORE_INPUTSOURCEMANAGER_HPP
#define _CORE_INPUTSOURCEMANAGER_HPP

#include "core/InputSource.hpp"

class InputSourceManager {
  public:
    InputSourceManager(InputSource **input_sources, size_t input_source_count);

    void ScanInputs(InputState &inputs);
    void ScanInputs(InputState &inputs, InputScanSpeed input_source_filter);

  private:
    InputSource **_input_sources;
    size_t _input_source_count;
};

#endif
