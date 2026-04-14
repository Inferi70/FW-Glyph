#ifndef _GLYPH_OVERRIDES_HPP
#define _GLYPH_OVERRIDES_HPP

#include "comms/B0XXInputViewer.hpp"
#include "comms/IntegratedDisplay.hpp"
#include "comms/NeoPixelBackend.hpp"
#include "core/config_utils.hpp"
#include "neopixel_definitions.hpp"
#include "stdlib.hpp"

#include <Wire.h>
#include <config.pb.h>

// clang-format off

const Config default_config = {
    .game_mode_configs_count = 14,
    .game_mode_configs = {
        GameModeConfig {
            .mode_id = MODE_FGC,
            .name = "Strive",
            .socd_pairs_count = 2,
            .socd_pairs = {
                SocdPair { .button_dir1 = BTN_LF3, .button_dir2 = BTN_LF1, .socd_type = SOCD_NEUTRAL },
                SocdPair { .button_dir1 = BTN_LF2, .button_dir2 = BTN_LT1, .socd_type = SOCD_2IP },
            },
            .button_remapping_count = 33,
            .button_remapping = {
                ButtonRemap { .physical_button = BTN_RF1,  .activates = BTN_RF6 },
                ButtonRemap { .physical_button = BTN_RF6,  .activates = BTN_RF1 },
                ButtonRemap { .physical_button = BTN_RF8,  .activates = BTN_LT2 },
                ButtonRemap { .physical_button = BTN_LF8,  .activates = BTN_LF3 },
                ButtonRemap { .physical_button = BTN_LF7,  .activates = BTN_LF2 },
                ButtonRemap { .physical_button = BTN_LF6,  .activates = BTN_LF1 },
                ButtonRemap { .physical_button = BTN_LT6,  .activates = BTN_LT1 },
                ButtonRemap { .physical_button = BTN_RT3,  .activates = BTN_RF4 },
                ButtonRemap { .physical_button = BTN_RT4,  .activates = BTN_RF3 },
                ButtonRemap { .physical_button = BTN_RF13, .activates = BTN_MB6 },

                ButtonRemap { .physical_button = BTN_LF1,  .activates = BTN_UNSPECIFIED },
                ButtonRemap { .physical_button = BTN_LF2,  .activates = BTN_UNSPECIFIED },
                ButtonRemap { .physical_button = BTN_LF3,  .activates = BTN_UNSPECIFIED },
                ButtonRemap { .physical_button = BTN_LF4,  .activates = BTN_UNSPECIFIED },
                ButtonRemap { .physical_button = BTN_LF5,  .activates = BTN_UNSPECIFIED },
                ButtonRemap { .physical_button = BTN_LT1,  .activates = BTN_UNSPECIFIED },
                ButtonRemap { .physical_button = BTN_LT2,  .activates = BTN_UNSPECIFIED },
                ButtonRemap { .physical_button = BTN_LT3,  .activates = BTN_UNSPECIFIED },
                ButtonRemap { .physical_button = BTN_LT4,  .activates = BTN_UNSPECIFIED },
                ButtonRemap { .physical_button = BTN_LT5,  .activates = BTN_UNSPECIFIED },
                ButtonRemap { .physical_button = BTN_RF9,  .activates = BTN_UNSPECIFIED },
                ButtonRemap { .physical_button = BTN_RF10, .activates = BTN_UNSPECIFIED },
                ButtonRemap { .physical_button = BTN_RF11, .activates = BTN_UNSPECIFIED },
                ButtonRemap { .physical_button = BTN_RF12, .activates = BTN_UNSPECIFIED },
                ButtonRemap { .physical_button = BTN_RF14, .activates = BTN_UNSPECIFIED },
                ButtonRemap { .physical_button = BTN_RF15, .activates = BTN_UNSPECIFIED },
                ButtonRemap { .physical_button = BTN_RF16, .activates = BTN_UNSPECIFIED },
                ButtonRemap { .physical_button = BTN_RT1,  .activates = BTN_UNSPECIFIED },
                ButtonRemap { .physical_button = BTN_RT2,  .activates = BTN_UNSPECIFIED },
                ButtonRemap { .physical_button = BTN_RT5,  .activates = BTN_UNSPECIFIED },
                ButtonRemap { .physical_button = BTN_MB1,  .activates = BTN_UNSPECIFIED },
                ButtonRemap { .physical_button = BTN_MB2,  .activates = BTN_UNSPECIFIED },
                ButtonRemap { .physical_button = BTN_MB3,  .activates = BTN_UNSPECIFIED },
            },
            .rgb_config = 5,
            .layout_plate = LAYOUT_PLATE_EVERYTHING,
            .applicable_backends_count = 5,
            .applicable_backends = {
                COMMS_BACKEND_XINPUT,
                COMMS_BACKEND_DINPUT,
                COMMS_BACKEND_NINTENDO_SWITCH,
                COMMS_BACKEND_PASSTHROUGH_PS4,
                COMMS_BACKEND_PASSTHROUGH_PS5,
            },
            .menu_button_icon_count = 7,
            .menu_button_icon = {
                OUT_UNSPECIFIED,
                OUT_UNSPECIFIED,
                OUT_UNSPECIFIED,
                OUT_UNSPECIFIED,
                OUT_HOME,
                OUT_XB_BACK,
                OUT_START,
            }
        },
        GameModeConfig {
            .mode_id = MODE_MELEE,
            .name = "Melee",
            .socd_pairs_count = 4,
            .socd_pairs = {
                SocdPair { .button_dir1 = BTN_LF3, .button_dir2 = BTN_LF1, .socd_type = SOCD_2IP_NO_REAC },
                SocdPair { .button_dir1 = BTN_LF2, .button_dir2 = BTN_RF4, .socd_type = SOCD_2IP_NO_REAC },
                SocdPair { .button_dir1 = BTN_RT3, .button_dir2 = BTN_RT5, .socd_type = SOCD_2IP_NO_REAC },
                SocdPair { .button_dir1 = BTN_RT2, .button_dir2 = BTN_RT4, .socd_type = SOCD_2IP_NO_REAC },
            },
            .button_remapping_count = 5,
            .button_remapping = {
                ButtonRemap { .physical_button = BTN_MB1,  .activates = BTN_UNSPECIFIED }, // Menu
            
                ButtonRemap { .physical_button = BTN_LF8, .activates = BTN_UNSPECIFIED },
                ButtonRemap { .physical_button = BTN_LF7, .activates = BTN_UNSPECIFIED },
                ButtonRemap { .physical_button = BTN_LF6, .activates = BTN_UNSPECIFIED },
                ButtonRemap { .physical_button = BTN_LT6, .activates = BTN_UNSPECIFIED },
            },
            .rgb_config = 1,
            .layout_plate = LAYOUT_PLATE_EVERYTHING,
            .applicable_backends_count = 6,
            .applicable_backends = {
                COMMS_BACKEND_XINPUT,
                COMMS_BACKEND_DINPUT,
                COMMS_BACKEND_NINTENDO_SWITCH,
                COMMS_BACKEND_PASSTHROUGH_PS4,
                COMMS_BACKEND_PASSTHROUGH_PS5,
                COMMS_BACKEND_GAMECUBE,
            },
            .menu_button_icon_count = 7,
            .menu_button_icon = {
                OUT_UNSPECIFIED,
                OUT_UNSPECIFIED,
                OUT_UNSPECIFIED,
                OUT_UNSPECIFIED,
                OUT_HOME,
                OUT_XB_BACK,
                OUT_START,
            }
        },
        GameModeConfig {
            .mode_id = MODE_PROJECT_M,
            .name = "Brawl",
            .socd_pairs_count = 4,
            .socd_pairs = {
                SocdPair { .button_dir1 = BTN_LF3, .button_dir2 = BTN_LF1, .socd_type = SOCD_2IP_NO_REAC },
                SocdPair { .button_dir1 = BTN_LF2, .button_dir2 = BTN_RF4, .socd_type = SOCD_2IP_NO_REAC },
                SocdPair { .button_dir1 = BTN_RT3, .button_dir2 = BTN_RT5, .socd_type = SOCD_2IP_NO_REAC },
                SocdPair { .button_dir1 = BTN_RT2, .button_dir2 = BTN_RT4, .socd_type = SOCD_2IP_NO_REAC },
            },
            .button_remapping_count = 6,
            .button_remapping = {
                ButtonRemap { .physical_button = BTN_MB1,  .activates = BTN_UNSPECIFIED }, // Menu
            
                ButtonRemap { .physical_button = BTN_LF8, .activates = BTN_UNSPECIFIED },
                ButtonRemap { .physical_button = BTN_LF7, .activates = BTN_UNSPECIFIED },
                ButtonRemap { .physical_button = BTN_LF6, .activates = BTN_UNSPECIFIED },
                ButtonRemap { .physical_button = BTN_LT6, .activates = BTN_UNSPECIFIED },

                ButtonRemap { .physical_button = BTN_RF9, .activates = BTN_UNSPECIFIED },
            },
            .rgb_config = 2,
            .layout_plate = LAYOUT_PLATE_EVERYTHING,
            .applicable_backends_count = 6,
            .applicable_backends = {
                COMMS_BACKEND_XINPUT,
                COMMS_BACKEND_DINPUT,
                COMMS_BACKEND_NINTENDO_SWITCH,
                COMMS_BACKEND_PASSTHROUGH_PS4,
                COMMS_BACKEND_PASSTHROUGH_PS5,
                COMMS_BACKEND_GAMECUBE,
            },
            .menu_button_icon_count = 7,
            .menu_button_icon = {
                OUT_UNSPECIFIED,
                OUT_UNSPECIFIED,
                OUT_UNSPECIFIED,
                OUT_UNSPECIFIED,
                OUT_HOME,
                OUT_XB_BACK,
                OUT_START,
            }
        },
        GameModeConfig {
            .mode_id = MODE_ULTIMATE,
            .name = "Ultimate",
            .socd_pairs_count = 4,
            .socd_pairs = {
                SocdPair { .button_dir1 = BTN_LF3, .button_dir2 = BTN_LF1, .socd_type = SOCD_2IP },
                SocdPair { .button_dir1 = BTN_LF2, .button_dir2 = BTN_RF4, .socd_type = SOCD_2IP },
                SocdPair { .button_dir1 = BTN_RT3, .button_dir2 = BTN_RT5, .socd_type = SOCD_2IP },
                SocdPair { .button_dir1 = BTN_RT2, .button_dir2 = BTN_RT4, .socd_type = SOCD_2IP },
            },
            .button_remapping_count = 5,
            .button_remapping = {
                ButtonRemap { .physical_button = BTN_MB1,  .activates = BTN_UNSPECIFIED }, // Menu
            
                ButtonRemap { .physical_button = BTN_LF8, .activates = BTN_UNSPECIFIED },
                ButtonRemap { .physical_button = BTN_LF7, .activates = BTN_UNSPECIFIED },
                ButtonRemap { .physical_button = BTN_LF6, .activates = BTN_UNSPECIFIED },
                ButtonRemap { .physical_button = BTN_LT6, .activates = BTN_UNSPECIFIED },
            },
            .rgb_config = 3,
            .layout_plate = LAYOUT_PLATE_EVERYTHING,
            .applicable_backends_count = 6,
            .applicable_backends = {
                COMMS_BACKEND_XINPUT,
                COMMS_BACKEND_DINPUT,
                COMMS_BACKEND_NINTENDO_SWITCH,
                COMMS_BACKEND_PASSTHROUGH_PS4,
                COMMS_BACKEND_PASSTHROUGH_PS5,
                COMMS_BACKEND_GAMECUBE,
            },
            .menu_button_icon_count = 7,
            .menu_button_icon = {
                OUT_UNSPECIFIED,
                OUT_UNSPECIFIED,
                OUT_UNSPECIFIED,
                OUT_UNSPECIFIED,
                OUT_HOME,
                OUT_XB_BACK,
                OUT_START,
            }
        },
        GameModeConfig {
            .mode_id = MODE_FGC,
            .name = "Split FGC",
            .socd_pairs_count = 2,
            .socd_pairs = {
                SocdPair { .button_dir1 = BTN_LF3, .button_dir2 = BTN_LF1, .socd_type = SOCD_NEUTRAL },
                SocdPair { .button_dir1 = BTN_LF2, .button_dir2 = BTN_LT1, .socd_type = SOCD_NEUTRAL },
            },
            .button_remapping_count = 13,
            .button_remapping = {
                ButtonRemap { .physical_button = BTN_RT1,  .activates = BTN_LT1 }, // Right thumb = Up
                ButtonRemap { .physical_button = BTN_LF5,  .activates = BTN_LT2 }, // WASD S key = LS Click
                ButtonRemap { .physical_button = BTN_RF9,  .activates = BTN_RT1 }, // Third row RF key = RS Click

                ButtonRemap { .physical_button = BTN_LF8, .activates = BTN_UNSPECIFIED },
                ButtonRemap { .physical_button = BTN_LF7, .activates = BTN_UNSPECIFIED },
                ButtonRemap { .physical_button = BTN_LF6, .activates = BTN_UNSPECIFIED },
                ButtonRemap { .physical_button = BTN_LT6, .activates = BTN_UNSPECIFIED },
                ButtonRemap { .physical_button = BTN_LT2,  .activates = BTN_UNSPECIFIED },

                ButtonRemap { .physical_button = BTN_RT2,  .activates = BTN_UNSPECIFIED },
                ButtonRemap { .physical_button = BTN_RT3,  .activates = BTN_UNSPECIFIED },
                ButtonRemap { .physical_button = BTN_RT4,  .activates = BTN_UNSPECIFIED },
                ButtonRemap { .physical_button = BTN_RT5,  .activates = BTN_UNSPECIFIED },
                ButtonRemap { .physical_button = BTN_MB1,  .activates = BTN_UNSPECIFIED },
            },
            .rgb_config = 4,
            .layout_plate = LAYOUT_PLATE_EVERYTHING,
            .applicable_backends_count = 5,
            .applicable_backends = {
                COMMS_BACKEND_XINPUT,
                COMMS_BACKEND_DINPUT,
                COMMS_BACKEND_NINTENDO_SWITCH,
                COMMS_BACKEND_PASSTHROUGH_PS4,
                COMMS_BACKEND_PASSTHROUGH_PS5,
            },
            .menu_button_icon_count = 7,
            .menu_button_icon = {
                OUT_UNSPECIFIED,
                OUT_UNSPECIFIED,
                OUT_UNSPECIFIED,
                OUT_UNSPECIFIED,
                OUT_HOME,
                OUT_XB_BACK,
                OUT_START,
            }
        },
        GameModeConfig {
            .mode_id = MODE_FGC,
            .name = "FGC",
            .socd_pairs_count = 2,
            .socd_pairs = {
                SocdPair { .button_dir1 = BTN_LF3, .button_dir2 = BTN_LF1, .socd_type = SOCD_NEUTRAL },
                SocdPair { .button_dir1 = BTN_LF2, .button_dir2 = BTN_LT1, .socd_type = SOCD_NEUTRAL },
            },
            .button_remapping_count = 31,
            .button_remapping = {
                // Right hand bottom row
                ButtonRemap { .physical_button = BTN_RF10,  .activates = BTN_RF1 },
                ButtonRemap { .physical_button = BTN_RF11, .activates = BTN_RF2 },
                ButtonRemap { .physical_button = BTN_RF12, .activates = BTN_RF3 },
                ButtonRemap { .physical_button = BTN_RF1,  .activates = BTN_RF4 },
                // Right hand top row
                ButtonRemap { .physical_button = BTN_RF13, .activates = BTN_RF5 },
                ButtonRemap { .physical_button = BTN_RF14, .activates = BTN_RF6 },
                ButtonRemap { .physical_button = BTN_RF15, .activates = BTN_RF7 },
                ButtonRemap { .physical_button = BTN_RF5,  .activates = BTN_RF8 },
                // Left hand row
                ButtonRemap { .physical_button = BTN_LF6,  .activates = BTN_LF1 },
                ButtonRemap { .physical_button = BTN_LF7,  .activates = BTN_LF2 },
                ButtonRemap { .physical_button = BTN_LF8,  .activates = BTN_LF3 },
                // Up button
                ButtonRemap { .physical_button = BTN_LT6,  .activates = BTN_LT1 },
                // Menu buttons
                // ButtonRemap { .physical_button = BTN_MB3,  .activates = BTN_RT3 },
                // ButtonRemap { .physical_button = BTN_MB4,  .activates = BTN_RT2 },
                // ButtonRemap { .physical_button = BTN_MB2,  .activates = BTN_MB1 },
                // Extra buttons
                ButtonRemap { .physical_button = BTN_RF16, .activates = BTN_LT2 },
                
                // Unmap the old buttons
                ButtonRemap { .physical_button = BTN_RF2,  .activates = BTN_UNSPECIFIED },
                ButtonRemap { .physical_button = BTN_RF3,  .activates = BTN_UNSPECIFIED },
                ButtonRemap { .physical_button = BTN_RF4,  .activates = BTN_UNSPECIFIED },
                ButtonRemap { .physical_button = BTN_RF6,  .activates = BTN_UNSPECIFIED },
                ButtonRemap { .physical_button = BTN_RF7,  .activates = BTN_UNSPECIFIED },
                ButtonRemap { .physical_button = BTN_RF8,  .activates = BTN_UNSPECIFIED },
                ButtonRemap { .physical_button = BTN_LF1,  .activates = BTN_UNSPECIFIED },
                ButtonRemap { .physical_button = BTN_LF2,  .activates = BTN_UNSPECIFIED },
                ButtonRemap { .physical_button = BTN_LF3,  .activates = BTN_UNSPECIFIED },
                ButtonRemap { .physical_button = BTN_LF5,  .activates = BTN_UNSPECIFIED },
                ButtonRemap { .physical_button = BTN_LT1,  .activates = BTN_UNSPECIFIED },
                ButtonRemap { .physical_button = BTN_LT2,  .activates = BTN_UNSPECIFIED },
                ButtonRemap { .physical_button = BTN_RT1,  .activates = BTN_UNSPECIFIED },
                ButtonRemap { .physical_button = BTN_RT2,  .activates = BTN_UNSPECIFIED },
                ButtonRemap { .physical_button = BTN_RT3,  .activates = BTN_UNSPECIFIED },
                ButtonRemap { .physical_button = BTN_RT4,  .activates = BTN_UNSPECIFIED },
                ButtonRemap { .physical_button = BTN_RT5,  .activates = BTN_UNSPECIFIED },
                ButtonRemap { .physical_button = BTN_MB1,  .activates = BTN_UNSPECIFIED },
            },
            .rgb_config = 5,
            .layout_plate = LAYOUT_PLATE_EVERYTHING,
            .applicable_backends_count = 5,
            .applicable_backends = {
                COMMS_BACKEND_XINPUT,
                COMMS_BACKEND_DINPUT,
                COMMS_BACKEND_NINTENDO_SWITCH,
                COMMS_BACKEND_PASSTHROUGH_PS4,
                COMMS_BACKEND_PASSTHROUGH_PS5,
            },
            .menu_button_icon_count = 7,
            .menu_button_icon = {
                OUT_UNSPECIFIED,
                OUT_UNSPECIFIED,
                OUT_UNSPECIFIED,
                OUT_UNSPECIFIED,
                OUT_HOME,
                OUT_XB_BACK,
                OUT_START,
            }
        },
        GameModeConfig {
            .mode_id = MODE_64,
            .name = "Smash64",
            .socd_pairs_count = 2,
            .socd_pairs = {
                SocdPair { .button_dir1 = BTN_LF3, .button_dir2 = BTN_LF1, .socd_type = SOCD_NEUTRAL },
                SocdPair { .button_dir1 = BTN_LF2, .button_dir2 = BTN_RF4, .socd_type = SOCD_NEUTRAL },
                SocdPair { .button_dir1 = BTN_RF7, .button_dir2 = BTN_RF8, .socd_type = SOCD_NEUTRAL },
                SocdPair { .button_dir1 = BTN_RF2, .button_dir2 = BTN_RF6, .socd_type = SOCD_NEUTRAL },
            },
            .button_remapping_count = 0,
            .rgb_config = 6,
            .layout_plate = LAYOUT_PLATE_EVERYTHING,
            .applicable_backends_count = 1,
            .applicable_backends = {
                COMMS_BACKEND_N64,
            },
            .menu_button_icon_count = 7,
            .menu_button_icon = {
                OUT_UNSPECIFIED,
                OUT_UNSPECIFIED,
                OUT_UNSPECIFIED,
                OUT_UNSPECIFIED,
                OUT_HOME,
                OUT_XB_BACK,
                OUT_START,
            }
        },

        GameModeConfig {
            .mode_id = MODE_RIVALS_OF_AETHER,
            .name = "RoA",
            .socd_pairs_count = 4,
            .socd_pairs = {
                SocdPair { .button_dir1 = BTN_LF3, .button_dir2 = BTN_LF1, .socd_type = SOCD_2IP },
                SocdPair { .button_dir1 = BTN_LF2, .button_dir2 = BTN_RF4, .socd_type = SOCD_2IP },
                SocdPair { .button_dir1 = BTN_RT3, .button_dir2 = BTN_RT5, .socd_type = SOCD_2IP },
                SocdPair { .button_dir1 = BTN_RT2, .button_dir2 = BTN_RT4, .socd_type = SOCD_2IP },
            },
            .button_remapping_count = 8,
            .button_remapping = {
                ButtonRemap { .physical_button = BTN_MB1,  .activates = BTN_UNSPECIFIED }, // Menu

                ButtonRemap { .physical_button = BTN_RF7, .activates = BTN_LF7 },
                ButtonRemap { .physical_button = BTN_RF8, .activates = BTN_LT6 },

                ButtonRemap { .physical_button = BTN_LF7, .activates = BTN_UNSPECIFIED },
                ButtonRemap { .physical_button = BTN_LT6, .activates = BTN_UNSPECIFIED },
                ButtonRemap { .physical_button = BTN_LF8, .activates = BTN_UNSPECIFIED },
                ButtonRemap { .physical_button = BTN_LF6, .activates = BTN_UNSPECIFIED },
                ButtonRemap { .physical_button = BTN_LT5, .activates = BTN_UNSPECIFIED },

            },
            .rgb_config = 7,
            .layout_plate = LAYOUT_PLATE_EVERYTHING,
            .applicable_backends_count = 6,
            .applicable_backends = {
                COMMS_BACKEND_XINPUT,
                COMMS_BACKEND_DINPUT,
                COMMS_BACKEND_NINTENDO_SWITCH,
                COMMS_BACKEND_PASSTHROUGH_PS4,
                COMMS_BACKEND_PASSTHROUGH_PS5,
                COMMS_BACKEND_GAMECUBE,
            },
            .menu_button_icon_count = 7,
            .menu_button_icon = {
                OUT_UNSPECIFIED,
                OUT_UNSPECIFIED,
                OUT_UNSPECIFIED,
                OUT_UNSPECIFIED,
                OUT_HOME,
                OUT_XB_BACK,
                OUT_START,
            }
        },

        GameModeConfig {
            .mode_id = MODE_RIVALS2,
            .name = "RoA2",
            .socd_pairs_count = 4,
            .socd_pairs = {
                SocdPair { .button_dir1 = BTN_LF3, .button_dir2 = BTN_LF1, .socd_type = SOCD_2IP },
                SocdPair { .button_dir1 = BTN_LF2, .button_dir2 = BTN_RF4, .socd_type = SOCD_2IP },
                SocdPair { .button_dir1 = BTN_RT3, .button_dir2 = BTN_RT5, .socd_type = SOCD_2IP },
                SocdPair { .button_dir1 = BTN_RT2, .button_dir2 = BTN_RT4, .socd_type = SOCD_2IP },
            },
            .button_remapping_count = 7,
            .button_remapping = {
                ButtonRemap { .physical_button = BTN_RF7, .activates = BTN_LF7 },
                ButtonRemap { .physical_button = BTN_RF8, .activates = BTN_LT6 },

                ButtonRemap { .physical_button = BTN_LF5, .activates = BTN_UNSPECIFIED },
                ButtonRemap { .physical_button = BTN_LF7, .activates = BTN_UNSPECIFIED },
                ButtonRemap { .physical_button = BTN_LT6, .activates = BTN_UNSPECIFIED },
                ButtonRemap { .physical_button = BTN_LF8, .activates = BTN_UNSPECIFIED },
                ButtonRemap { .physical_button = BTN_LF6, .activates = BTN_UNSPECIFIED },
            },
            .rgb_config = 8,
            .layout_plate = LAYOUT_PLATE_EVERYTHING,
            .applicable_backends_count = 6,
            .applicable_backends = {
                COMMS_BACKEND_XINPUT,
                COMMS_BACKEND_DINPUT,
                COMMS_BACKEND_NINTENDO_SWITCH,
                COMMS_BACKEND_PASSTHROUGH_PS4,
                COMMS_BACKEND_PASSTHROUGH_PS5,
                COMMS_BACKEND_GAMECUBE,
            },
            .menu_button_icon_count = 7,
            .menu_button_icon = {
                OUT_UNSPECIFIED,
                OUT_UNSPECIFIED,
                OUT_UNSPECIFIED,
                OUT_UNSPECIFIED,
                OUT_HOME,
                OUT_XB_BACK,
                OUT_START,
            }
        },

        GameModeConfig {
            .mode_id = MODE_MELEE,
            .name = "GameCube",
            .socd_pairs_count = 4,
            .socd_pairs = {
                SocdPair { .button_dir1 = BTN_LF3, .button_dir2 = BTN_LF1, .socd_type = SOCD_NEUTRAL },
                SocdPair { .button_dir1 = BTN_LF2, .button_dir2 = BTN_RF4, .socd_type = SOCD_NEUTRAL },
                SocdPair { .button_dir1 = BTN_RT3, .button_dir2 = BTN_RT5, .socd_type = SOCD_NEUTRAL },
                SocdPair { .button_dir1 = BTN_RT2, .button_dir2 = BTN_RT4, .socd_type = SOCD_NEUTRAL },
            },
            .button_remapping_count = 22,
            .button_remapping = {
                ButtonRemap { .physical_button = BTN_LF2, .activates = BTN_RF4 },
                ButtonRemap { .physical_button = BTN_LF6, .activates = BTN_LF8 },
                ButtonRemap { .physical_button = BTN_LF5, .activates = BTN_LF2 },
                ButtonRemap { .physical_button = BTN_RF13, .activates = BTN_LT6 },
                ButtonRemap { .physical_button = BTN_RF13, .activates = BTN_LT6 },
                ButtonRemap { .physical_button = BTN_RF10, .activates = BTN_LF7 },
                ButtonRemap { .physical_button = BTN_RF11, .activates = BTN_LF6 },

                ButtonRemap { .physical_button = BTN_LF7, .activates = BTN_UNSPECIFIED },
                ButtonRemap { .physical_button = BTN_LF8, .activates = BTN_UNSPECIFIED },
                ButtonRemap { .physical_button = BTN_LT3, .activates = BTN_UNSPECIFIED },
                ButtonRemap { .physical_button = BTN_LT4, .activates = BTN_UNSPECIFIED },
                ButtonRemap { .physical_button = BTN_LT5, .activates = BTN_UNSPECIFIED },
                ButtonRemap { .physical_button = BTN_LT6, .activates = BTN_UNSPECIFIED },
                ButtonRemap { .physical_button = BTN_RF4, .activates = BTN_UNSPECIFIED },
                ButtonRemap { .physical_button = BTN_RF9, .activates = BTN_UNSPECIFIED },
                ButtonRemap { .physical_button = BTN_RF12, .activates = BTN_UNSPECIFIED },
                ButtonRemap { .physical_button = BTN_RF14, .activates = BTN_UNSPECIFIED },
                ButtonRemap { .physical_button = BTN_RF15, .activates = BTN_UNSPECIFIED },
                ButtonRemap { .physical_button = BTN_RF16, .activates = BTN_UNSPECIFIED },
                ButtonRemap { .physical_button = BTN_MB1, .activates = BTN_UNSPECIFIED },
                ButtonRemap { .physical_button = BTN_MB2, .activates = BTN_UNSPECIFIED },
                ButtonRemap { .physical_button = BTN_MB3, .activates = BTN_UNSPECIFIED },
            },
            .rgb_config = 9,
            .layout_plate = LAYOUT_PLATE_EVERYTHING,
            .applicable_backends_count = 6,
            .applicable_backends = {
                COMMS_BACKEND_XINPUT,
                COMMS_BACKEND_DINPUT,
                COMMS_BACKEND_NINTENDO_SWITCH,
                COMMS_BACKEND_PASSTHROUGH_PS4,
                COMMS_BACKEND_PASSTHROUGH_PS5,
                COMMS_BACKEND_GAMECUBE,
            },
            .menu_button_icon_count = 7,
            .menu_button_icon = {
                OUT_UNSPECIFIED,
                OUT_UNSPECIFIED,
                OUT_UNSPECIFIED,
                OUT_UNSPECIFIED,
                OUT_HOME,
                OUT_XB_BACK,
                OUT_START,
            }
        },

        GameModeConfig {
            .mode_id = MODE_MELEE,
            .name = "N64",

            .socd_pairs_count = 4,
            .socd_pairs = {
                SocdPair { .button_dir1 = BTN_LF3, .button_dir2 = BTN_LF1, .socd_type = SOCD_NEUTRAL },
                SocdPair { .button_dir1 = BTN_LF2, .button_dir2 = BTN_RF4, .socd_type = SOCD_NEUTRAL },
                SocdPair { .button_dir1 = BTN_RT3, .button_dir2 = BTN_RT5, .socd_type = SOCD_NEUTRAL },
                SocdPair { .button_dir1 = BTN_RT2, .button_dir2 = BTN_RT4, .socd_type = SOCD_NEUTRAL },
            },

            .button_remapping_count = 24, 
            .button_remapping = {
                ButtonRemap { .physical_button = BTN_LF2,  .activates = BTN_RF4 },
                ButtonRemap { .physical_button = BTN_RF2,  .activates = BTN_RF5 },
                ButtonRemap { .physical_button = BTN_LF6,  .activates = BTN_LF8 },
                ButtonRemap { .physical_button = BTN_LF5,  .activates = BTN_LF2 },
                ButtonRemap { .physical_button = BTN_RF13,  .activates = BTN_LT6 },
                ButtonRemap { .physical_button = BTN_RF11,  .activates = BTN_LF6 },
                ButtonRemap { .physical_button = BTN_RF10,  .activates = BTN_LF7 },

                ButtonRemap { .physical_button = BTN_LF7,  .activates = BTN_UNSPECIFIED },
                ButtonRemap { .physical_button = BTN_LF8,  .activates = BTN_UNSPECIFIED },
                ButtonRemap { .physical_button = BTN_LT3,  .activates = BTN_UNSPECIFIED },
                ButtonRemap { .physical_button = BTN_LT4,  .activates = BTN_UNSPECIFIED },
                ButtonRemap { .physical_button = BTN_LT5,  .activates = BTN_UNSPECIFIED },
                ButtonRemap { .physical_button = BTN_LT6,  .activates = BTN_UNSPECIFIED },

                ButtonRemap { .physical_button = BTN_RF4,  .activates = BTN_UNSPECIFIED },
                ButtonRemap { .physical_button = BTN_RF5,  .activates = BTN_UNSPECIFIED },
                ButtonRemap { .physical_button = BTN_RF6,  .activates = BTN_UNSPECIFIED },
                ButtonRemap { .physical_button = BTN_RF7,  .activates = BTN_UNSPECIFIED },
                ButtonRemap { .physical_button = BTN_RF8,  .activates = BTN_UNSPECIFIED },
                ButtonRemap { .physical_button = BTN_RF9,  .activates = BTN_UNSPECIFIED },
                ButtonRemap { .physical_button = BTN_RF12,  .activates = BTN_UNSPECIFIED },
                ButtonRemap { .physical_button = BTN_RF14,  .activates = BTN_UNSPECIFIED },
                ButtonRemap { .physical_button = BTN_RF15,  .activates = BTN_UNSPECIFIED },
                ButtonRemap { .physical_button = BTN_RF16,  .activates = BTN_UNSPECIFIED },
                ButtonRemap { .physical_button = BTN_MB1,  .activates = BTN_UNSPECIFIED },
            },
            .rgb_config = 10,
            .layout_plate = LAYOUT_PLATE_EVERYTHING,
            .applicable_backends_count = 6,
            .applicable_backends = {
                COMMS_BACKEND_XINPUT,
                COMMS_BACKEND_DINPUT,
                COMMS_BACKEND_NINTENDO_SWITCH,
                COMMS_BACKEND_PASSTHROUGH_PS4,
                COMMS_BACKEND_PASSTHROUGH_PS5,
                COMMS_BACKEND_N64,
            },
            .menu_button_icon_count = 7,
            .menu_button_icon = {
                OUT_UNSPECIFIED,
                OUT_UNSPECIFIED,
                OUT_UNSPECIFIED,
                OUT_UNSPECIFIED,
                OUT_HOME,
                OUT_XB_BACK,
                OUT_START,
            }
        },
            
        GameModeConfig {
            .mode_id = MODE_FGC,
            .name = "SNES",
            .socd_pairs_count = 2,
            .socd_pairs = {
                SocdPair { .button_dir1 = BTN_LF3, .button_dir2 = BTN_LF1, .socd_type = SOCD_NEUTRAL },
                SocdPair { .button_dir1 = BTN_LF2, .button_dir2 = BTN_LT1, .socd_type = SOCD_NEUTRAL },
            },
            .button_remapping_count = 10,

            .button_remapping = {
                ButtonRemap { .physical_button = BTN_LF5, .activates = BTN_LF2 },
                ButtonRemap { .physical_button = BTN_LF2, .activates = BTN_LT1 },
                ButtonRemap { .physical_button = BTN_LT1, .activates = BTN_RF8 },
                ButtonRemap { .physical_button = BTN_RT1, .activates = BTN_RF7 },
                ButtonRemap { .physical_button = BTN_RF1, .activates = BTN_RF2 },
                ButtonRemap { .physical_button = BTN_RF2, .activates = BTN_RF1 },
                ButtonRemap { .physical_button = BTN_RF5, .activates = BTN_RF6 },
                ButtonRemap { .physical_button = BTN_RF6, .activates = BTN_RF5 },

                ButtonRemap { .physical_button = BTN_RF7,  .activates = BTN_UNSPECIFIED },
                ButtonRemap { .physical_button = BTN_RF8,  .activates = BTN_UNSPECIFIED },
            },
            .rgb_config = 11,
            .layout_plate = LAYOUT_PLATE_EVERYTHING,
            .applicable_backends_count = 1,
            .applicable_backends = {
                COMMS_BACKEND_SNES,
            },
            .menu_button_icon_count = 7,
            .menu_button_icon = {
                OUT_UNSPECIFIED,
                OUT_UNSPECIFIED,
                OUT_UNSPECIFIED,
                OUT_UNSPECIFIED,
                OUT_HOME,
                OUT_XB_BACK,
                OUT_START,
            }
        },

        GameModeConfig {
            .mode_id = MODE_FGC,
            .name = "NES",
            .socd_pairs_count = 2,
            .socd_pairs = {
                SocdPair { .button_dir1 = BTN_LF3, .button_dir2 = BTN_LF1, .socd_type = SOCD_NEUTRAL },
                SocdPair { .button_dir1 = BTN_LF2, .button_dir2 = BTN_LT1, .socd_type = SOCD_NEUTRAL },
            },
            .button_remapping_count = 8,
            .button_remapping = {

                ButtonRemap { .physical_button = BTN_LF2, .activates = BTN_LT1 },
                ButtonRemap { .physical_button = BTN_RF1, .activates = BTN_RF2 },
                ButtonRemap { .physical_button = BTN_RF2, .activates = BTN_RF1 },
                ButtonRemap { .physical_button = BTN_LF5, .activates = BTN_LF2 },

                // Unmap the old buttons
                ButtonRemap { .physical_button = BTN_LF2,  .activates = BTN_UNSPECIFIED },
                ButtonRemap { .physical_button = BTN_LT1,  .activates = BTN_UNSPECIFIED },
                ButtonRemap { .physical_button = BTN_RF2,  .activates = BTN_UNSPECIFIED },
                ButtonRemap { .physical_button = BTN_RF1,  .activates = BTN_UNSPECIFIED },
            },
            .rgb_config = 12,
            .layout_plate = LAYOUT_PLATE_EVERYTHING,
            .applicable_backends_count = 1,
            .applicable_backends = {
                COMMS_BACKEND_NES,
            },
            .menu_button_icon_count = 7,
            .menu_button_icon = {
                OUT_UNSPECIFIED,
                OUT_UNSPECIFIED,
                OUT_UNSPECIFIED,
                OUT_HOME,
                OUT_XB_BACK,
                OUT_UNSPECIFIED,
            }
        },
        GameModeConfig {
            .mode_id = MODE_KEYBOARD,
            .socd_pairs_count = 2,
            .socd_pairs = {
                SocdPair { .button_dir1 = BTN_LF3, .button_dir2 = BTN_LF1, .socd_type = SOCD_2IP },
                SocdPair { .button_dir1 = BTN_LT1, .button_dir2 = BTN_RT4, .socd_type = SOCD_2IP },
            },
            .button_remapping_count = 7,
            .button_remapping = {
                ButtonRemap { .physical_button = BTN_MB1,  .activates = BTN_UNSPECIFIED },
                ButtonRemap { .physical_button = BTN_MB2,  .activates = BTN_UNSPECIFIED },
                ButtonRemap { .physical_button = BTN_MB2,  .activates = BTN_UNSPECIFIED },
                ButtonRemap { .physical_button = BTN_MB3,  .activates = BTN_UNSPECIFIED },
                ButtonRemap { .physical_button = BTN_MB4,  .activates = BTN_UNSPECIFIED },
                ButtonRemap { .physical_button = BTN_MB5,  .activates = BTN_UNSPECIFIED },
                ButtonRemap { .physical_button = BTN_MB6,  .activates = BTN_UNSPECIFIED },
                ButtonRemap { .physical_button = BTN_MB7,  .activates = BTN_UNSPECIFIED },
            },
            .keyboard_mode_config = 1,
            .rgb_config = 11,
            .layout_plate = LAYOUT_PLATE_EVERYTHING,
            .applicable_backends_count = 1,
            .applicable_backends = {
                COMMS_BACKEND_DINPUT,
            },
            .menu_button_icon_count = 7,
            .menu_button_icon = {
                OUT_UNSPECIFIED,
                OUT_UNSPECIFIED,
                OUT_UNSPECIFIED,
                OUT_UNSPECIFIED,
                OUT_UNSPECIFIED,
                OUT_UNSPECIFIED,
                OUT_UNSPECIFIED,
            }
        },
    },
    // 1  - Strive
    // 2  - Melee
    // 3  - PM
    // 4  - Ultimate
    // 5  - Split FGC
    // 6  - FGC
    // 7  - Smash64
    // 8  - RoA
    // 9  - RoA2
    // 10 - SNES
    // 11 - NES
    // 12 - Keyboard
    .communication_backend_configs_count = 10,
    .communication_backend_configs = {
        CommunicationBackendConfig {
            .backend_id = COMMS_BACKEND_XINPUT,
            .default_mode_config = 1,
        },
        CommunicationBackendConfig {
            .backend_id = COMMS_BACKEND_DINPUT,
            .default_mode_config = 1,
            .activation_binding_count = 1,
            .activation_binding = { BTN_RF3 },
        },
        CommunicationBackendConfig {
            .backend_id = COMMS_BACKEND_NINTENDO_SWITCH,
            .default_mode_config = 1,
            .activation_binding_count = 1,
            .activation_binding = { BTN_RF2 },
        },
        CommunicationBackendConfig {
            .backend_id = COMMS_BACKEND_PASSTHROUGH_PS4,
            .default_mode_config = 1,
        },
        CommunicationBackendConfig {
            .backend_id = COMMS_BACKEND_PASSTHROUGH_PS5,
            .default_mode_config = 1,
        },
        CommunicationBackendConfig {
            .backend_id = COMMS_BACKEND_GAMECUBE,
            .default_mode_config = 2,
        },
        CommunicationBackendConfig {
            .backend_id = COMMS_BACKEND_N64,
            .default_mode_config = 7,
        },
        CommunicationBackendConfig {
            .backend_id = COMMS_BACKEND_NES,
            .default_mode_config = 11,
            .activation_binding_count = 1,
            .activation_binding = { BTN_LT1 },
        },
        CommunicationBackendConfig {
            .backend_id = COMMS_BACKEND_SNES,
            .default_mode_config = 10,
            .activation_binding_count = 1,
            .activation_binding = { BTN_LT2 },
        },
        CommunicationBackendConfig {
            .backend_id = COMMS_BACKEND_CONFIGURATOR,
            .activation_binding_count = 1,
            .activation_binding = { BTN_RT2 },
        }
    },
    .keyboard_modes_count = 1,
    .keyboard_modes = {
        KeyboardModeConfig {
            0,
            35,
            {
                { BTN_LF1, HID_KEY_A },
                { BTN_LF2, HID_KEY_B },
                { BTN_LF3, HID_KEY_C },
                { BTN_LF4, HID_KEY_D },
                { BTN_LF5, HID_KEY_E },
                { BTN_LF6, HID_KEY_F },
                { BTN_LF7, HID_KEY_G },
                { BTN_LF8, HID_KEY_H },

                { BTN_LT1, HID_KEY_I },
                { BTN_LT2, HID_KEY_J },
                { BTN_LT3, HID_KEY_K },
                { BTN_LT4, HID_KEY_L },
                { BTN_LT5, HID_KEY_M },
                { BTN_LT6, HID_KEY_N },

                { BTN_RF1, HID_KEY_O },
                { BTN_RF2, HID_KEY_P },
                { BTN_RF3, HID_KEY_Q },
                { BTN_RF4, HID_KEY_R },
                { BTN_RF5, HID_KEY_S },
                { BTN_RF6, HID_KEY_T },
                { BTN_RF7, HID_KEY_U },
                { BTN_RF8, HID_KEY_V },
                { BTN_RF9, HID_KEY_W },
                { BTN_RF10, HID_KEY_X },
                { BTN_RF11, HID_KEY_Y },
                { BTN_RF12, HID_KEY_Z },
                { BTN_RF13, HID_KEY_1 },
                { BTN_RF14, HID_KEY_2 },
                { BTN_RF15, HID_KEY_3 },
                { BTN_RF16, HID_KEY_4 },

                { BTN_RT1, HID_KEY_5 },
                { BTN_RT2, HID_KEY_6 },
                { BTN_RT3, HID_KEY_7 },
                { BTN_RT4, HID_KEY_8 },
                { BTN_RT5, HID_KEY_9 },

            },
        },
    },
    .rgb_configs_count = 13,
    .rgb_configs = {
        RgbConfig {
            .button_colors_count = 20,
            .button_colors = {
                {
                    BTN_LF1,
                    2282478
                },
                {
                    BTN_LF2,
                    2282478
                },
                {
                    BTN_LF3,
                    2282478
                },
                {
                    BTN_LF4,
                    2282478
                },
                {
                    BTN_LT1,
                    2282478
                },
                {
                    BTN_LT2,
                    2282478
                },
                {
                    BTN_RF1,
                    2282478
                },
                {
                    BTN_RF2,
                    2282478
                },
                {
                    BTN_RF3,
                    2282478
                },
                {
                    BTN_RF4,
                    2282478
                },
                {
                    BTN_RF5,
                    2282478
                },
                {
                    BTN_RF6,
                    2282478
                },
                {
                    BTN_RF7,
                    2282478
                },
                {
                    BTN_RF8,
                    2282478
                },
                {
                    BTN_RT1,
                    2282478
                },
                {
                    BTN_RT2,
                    2282478
                },
                {
                    BTN_RT3,
                    2282478
                },
                {
                    BTN_RT4,
                    2282478
                },
                {
                    BTN_RT5,
                    2282478
                },
                {
                    BTN_MB1,
                    2282478
                },
            },
            .animation = RGB_ANIM_STATIC,
        },

        RgbConfig {
          .button_colors_count = 20,
          .button_colors = {
                {
                BTN_LF1,
                2282478
                },
                {
                BTN_LF2,
                2282478
                },
                {
                BTN_LF3,
                2282478
                },
                {
                BTN_LF4,
                2282478
                },
                {
                BTN_LT1,
                2282478
                },
                {
                BTN_LT2,
                2282478
                },
                {
                BTN_RF1,
                2282478
                },
                {
                BTN_RF2,
                2282478
                },
                {
                BTN_RF3,
                2282478
                },
                {
                BTN_RF4,
                2282478
                },
                {
                BTN_RF5,
                2282478
                },
                {
                BTN_RF6,
                2282478
                },
                {
                BTN_RF7,
                2282478
                },
                {
                BTN_RF8,
                2282478
                },
                {
                BTN_RT1,
                2282478
                },
                {
                BTN_RT2,
                2282478
                },
                {
                BTN_RT3,
                2282478
                },
                {
                BTN_RT4,
                2282478
                },
                {
                BTN_RT5,
                2282478
                },
                {
                BTN_MB1,
                2282478
                }
          },
          .animation = RGB_ANIM_STATIC
        },

        RgbConfig {
          .button_colors_count = 20,
          .button_colors = {
                {
                BTN_LF1,
                2282478
                },
                {
                BTN_LF2,
                2282478
                },
                {
                BTN_LF3,
                2282478
                },
                {
                BTN_LF4,
                2282478
                },
                {
                BTN_LT1,
                2282478
                },
                {
                BTN_LT2,
                2282478
                },
                {
                BTN_RF1,
                2282478
                },
                {
                BTN_RF2,
                2282478
                },
                {
                BTN_RF3,
                2282478
                },
                {
                BTN_RF4,
                2282478
                },
                {
                BTN_RF5,
                2282478
                },
                {
                BTN_RF6,
                2282478
                },
                {
                BTN_RF7,
                2282478
                },
                {
                BTN_RF8,
                2282478
                },
                {
                BTN_RT1,
                2282478
                },
                {
                BTN_RT2,
                2282478
                },
                {
                BTN_RT3,
                2282478
                },
                {
                BTN_RT4,
                2282478
                },
                {
                BTN_RT5,
                2282478
                },
                {
                BTN_MB1,
                2282478
                }
          },
          .animation = RGB_ANIM_STATIC
        },


        RgbConfig {
          .button_colors_count = 16,
          .button_colors = {
            {
              BTN_LF1,
              2282478
            },
            {
              BTN_LF2,
              2282478
            },
            {
              BTN_LF3,
              2282478
            },
            {
              BTN_LF5,
              2282478
            },
            {
              BTN_LT1,
              2282478
            },
            {
              BTN_RF1,
              2282478
            },
            {
              BTN_RF2,
              2282478
            },
            {
              BTN_RF3,
              2282478
            },
            {
              BTN_RF4,
              2282478
            },
            {
              BTN_RF5,
              2282478
            },
            {
              BTN_RF6,
              2282478
            },
            {
              BTN_RF7,
              2282478
            },
            {
              BTN_RF8,
              2282478
            },
            {
              BTN_RF9,
              2282478
            },
            {
              BTN_RT1,
              2282478
            },
            {
              BTN_MB1,
              2282478
            }
          },
          .animation = RGB_ANIM_STATIC
        },


        RgbConfig {
          .button_colors_count = 14,
          .button_colors = {
                {
                BTN_LF8,
                2282478
                },
                {
                BTN_LF7,
                2282478
                },
                {
                BTN_LF6,
                2282478
                },
                {
                BTN_LT6,
                2282478
                },
                {
                BTN_RF10,
                2282478
                },
                {
                BTN_RF11,
                2282478
                },
                {
                BTN_RF12,
                2282478
                },
                {
                BTN_RF1,
                2282478
                },
                {
                BTN_RF13,
                2282478
                },
                {
                BTN_RF14,
                2282478
                },
                {
                BTN_RF15,
                2282478
                },
                {
                BTN_RF5,
                2282478
                },
                {
                BTN_RF16,
                2282478
                },
                {
                BTN_MB1,
                2282478
                }
            },
            .animation = RGB_ANIM_STATIC
        },

        RgbConfig {
          .button_colors_count = 16,
          .button_colors = {
            {
              BTN_LF1,
              2282478
            },
            {
              BTN_LF2,
              2282478
            },
            {
              BTN_LF3,
              2282478
            },
            {
              BTN_LF4,
              2282478
            },
            {
              BTN_LT1,
              2282478
            },
            {
              BTN_LT2,
              2282478
            },
            {
              BTN_RF1,
              2282478
            },
            {
              BTN_RF2,
              2282478
            },
            {
              BTN_RF3,
              2282478
            },
            {
              BTN_RF4,
              2282478
            },
            {
              BTN_RF5,
              2282478
            },
            {
              BTN_RF6,
              2282478
            },
            {
              BTN_RT1,
              2282478
            },
            {
              BTN_MB1,
              2282478
            },
            {
              BTN_RF7,
              2282478
            },
            {
              BTN_RF8,
              2282478
            }
          },
          .animation = RGB_ANIM_STATIC,
        },


        RgbConfig {
          .button_colors_count = 20,
          .button_colors = {
                {
                BTN_LF1,
                2282478
                },
                {
                BTN_LF2,
                2282478
                },
                {
                BTN_LF3,
                2282478
                },
                {
                BTN_LF4,
                2282478
                },
                {
                BTN_LT1,
                2282478
                },
                {
                BTN_LT2,
                2282478
                },
                {
                BTN_RF1,
                2282478
                },
                {
                BTN_RF2,
                2282478
                },
                {
                BTN_RF3,
                2282478
                },
                {
                BTN_RF4,
                2282478
                },
                {
                BTN_RF5,
                2282478
                },
                {
                BTN_RF6,
                2282478
                },
                {
                BTN_RF7,
                2282478
                },
                {
                BTN_RF8,
                2282478
                },
                {
                BTN_RT1,
                2282478
                },
                {
                BTN_RT2,
                2282478
                },
                {
                BTN_RT3,
                2282478
                },
                {
                BTN_RT4,
                2282478
                },
                {
                BTN_RT5,
                2282478
                },
                {
                BTN_MB1,
                2282478
                }
            },
            .animation = RGB_ANIM_STATIC
        },


        RgbConfig {
          .button_colors_count = 20,
          .button_colors = {
                {
                BTN_LF1,
                2282478
                },
                {
                BTN_LF2,
                2282478
                },
                {
                BTN_LF3,
                2282478
                },
                {
                BTN_LF4,
                2282478
                },
                {
                BTN_LT1,
                2282478
                },
                {
                BTN_LT2,
                2282478
                },
                {
                BTN_RF1,
                2282478
                },
                {
                BTN_RF2,
                2282478
                },
                {
                BTN_RF3,
                2282478
                },
                {
                BTN_RF4,
                2282478
                },
                {
                BTN_RF5,
                2282478
                },
                {
                BTN_RF6,
                2282478
                },
                {
                BTN_RF7,
                2282478
                },
                {
                BTN_RF8,
                2282478
                },
                {
                BTN_RT1,
                2282478
                },
                {
                BTN_RT2,
                2282478
                },
                {
                BTN_RT3,
                2282478
                },
                {
                BTN_RT4,
                2282478
                },
                {
                BTN_RT5,
                2282478
                },
                {
                BTN_MB1,
                2282478
                }
            },
            .animation = RGB_ANIM_STATIC
        },

        RgbConfig {
          .button_colors_count = 24,
          .button_colors = {
                {
                BTN_LF1,
                2282478
                },
                {
                BTN_LF2,
                2282478
                },
                {
                BTN_LF3,
                2282478
                },
                {
                BTN_LF4,
                2282478
                },
                {
                BTN_LT1,
                2282478
                },
                {
                BTN_LT2,
                2282478
                },
                {
                BTN_RF1,
                2282478
                },
                {
                BTN_RF2,
                2282478
                },
                {
                BTN_RF3,
                2282478
                },
                {
                BTN_RF5,
                2282478
                },
                {
                BTN_RF6,
                2282478
                },
                {
                BTN_RF7,
                2282478
                },
                {
                BTN_RF8,
                2282478
                },
                {
                BTN_RT1,
                2282478
                },
                {
                BTN_RT2,
                2282478
                },
                {
                BTN_RT3,
                2282478
                },
                {
                BTN_RT4,
                2282478
                },
                {
                BTN_RT5,
                2282478
                },
                {
                BTN_MB1,
                2282478
                },
                {
                BTN_LF6,
                2282478
                },
                {
                BTN_LF5,
                2282478
                },
                {
                BTN_RF13,
                2282478
                },
                {
                BTN_RF10,
                2282478
                },
                {
                BTN_RF11,
                2282478
                },
            },
            .animation = RGB_ANIM_STATIC
        },

        RgbConfig {
          .button_colors_count = 20,
          .button_colors = {
                {
                BTN_LF1,
                2282478
                },
                {
                BTN_LF2,
                2282478
                },
                {
                BTN_LF3,
                2282478
                },
                {
                BTN_LF4,
                2282478
                },
                {
                BTN_LT1,
                2282478
                },
                {
                BTN_LT2,
                2282478
                },
                {
                BTN_RF1,
                2282478
                },
                {
                BTN_RF2,
                2282478
                },
                {
                BTN_RF3,
                2282478
                },
                {
                BTN_RT1,
                2282478
                },
                {
                BTN_RT2,
                2282478
                },
                {
                BTN_RT3,
                2282478
                },
                {
                BTN_RT4,
                2282478
                },
                {
                BTN_RT5,
                2282478
                },
                {
                BTN_MB1,
                2282478
                },
                {
                BTN_LF5,
                2282478
                },
                {
                BTN_RF13,
                2282478
                },
                {
                BTN_LF6,
                2282478
                },
                {
                BTN_RF11,
                2282478
                },
                {
                BTN_RF10,
                2282478
                }
            },
            .animation = RGB_ANIM_STATIC
        },

        RgbConfig {
          .button_colors_count = 20,
          .button_colors = {
                {
                BTN_LF1,
                2282478
                },
                {
                BTN_LF2,
                2282478
                },
                {
                BTN_LF3,
                2282478
                },
                {
                BTN_LT1,
                2282478
                },
                {
                BTN_RF1,
                2282478
                },
                {
                BTN_RF2,
                2282478
                },
                {
                BTN_RF5,
                2282478
                },
                {
                BTN_RF6,
                2282478
                },
                {
                BTN_RT1,
                2282478
                },
                {
                BTN_MB1,
                2282478
                },
                {
                BTN_LF5,
                2282478
                },
            },
            .animation = RGB_ANIM_STATIC
        },

        RgbConfig {
          .button_colors_count = 7,
          .button_colors = {
                {
                BTN_LF1,
                2282478
                },
                {
                BTN_LF2,
                2282478
                },
                {
                BTN_LF3,
                2282478
                },
                {
                BTN_LF5,
                2282478
                },
                {
                BTN_RF1,
                2282478
                },
                {
                BTN_RF2,
                2282478
                },
                {
                BTN_MB1,
                2282478
                },
            },
          .animation = RGB_ANIM_STATIC
        },

        RgbConfig {
          .button_colors_count = 36,
          .button_colors = {
                {
                BTN_LF1,
                2282478
                },
                {
                BTN_LF2,
                2282478
                },
                {
                BTN_LF3,
                2282478
                },
                {
                BTN_LF4,
                2282478
                },
                {
                BTN_LF5,
                2282478
                },
                {
                BTN_LF6,
                2282478
                },
                {
                BTN_LF7,
                2282478
                },
                {
                BTN_LF8,
                2282478
                },
                {
                BTN_LT1,
                2282478
                },
                {
                BTN_LT2,
                2282478
                },
                {
                BTN_LT3,
                2282478
                },
                {
                BTN_LT4,
                2282478
                },
                {
                BTN_LT5,
                2282478
                },
                {
                BTN_LT6,
                2282478
                },
                {
                BTN_RF1,
                2282478
                },
                {
                BTN_RF2,
                2282478
                },
                {
                BTN_RF3,
                2282478
                },
                {
                BTN_RF4,
                2282478
                },
                {
                BTN_RF5,
                2282478
                },
                {
                BTN_RF6,
                2282478
                },
                {
                BTN_RF7,
                2282478
                },
                {
                BTN_RF8,
                2282478
                },
                {
                BTN_RF9,
                2282478
                },
                {
                BTN_RF10,
                2282478
                },
                {
                BTN_RF11,
                2282478
                },
                {
                BTN_RF12,
                2282478
                },
                {
                BTN_RF13,
                2282478
                },
                {
                BTN_RF14,
                2282478
                },
                {
                BTN_RF15,
                2282478
                },
                {
                BTN_RF16,
                2282478
                },
                {
                BTN_RT1,
                2282478
                },
                {
                BTN_RT2,
                2282478
                },
                {
                BTN_RT3,
                2282478
                },
                {
                BTN_RT4,
                2282478
                },
                {
                BTN_RT5,
                2282478
                },
                {
                BTN_MB1,
                2282478
                }
          },
          .animation = RGB_ANIM_STATIC
        }
    },
    .default_backend_config = 1,
    .default_usb_backend_config = 1,
    .rgb_brightness = 255,
};

// clang-format on

Config glyph_default_config() {
    Config config = default_config;

    // Not needed, these are set in the defaultConfig struct.

    // Assign layout plates and applicable backends for default gamemode configs.
    // for (size_t i = 0; i < config.game_mode_configs_count; i++) {
    //     GameModeConfig &mode_config = config.game_mode_configs[i];
    //     switch (mode_config.mode_id) {
    //         case MODE_FGC:
    //             mode_config.applicable_backends[0] = COMMS_BACKEND_XINPUT;
    //             mode_config.applicable_backends[1] = COMMS_BACKEND_DINPUT;
    //             mode_config.applicable_backends[2] = COMMS_BACKEND_NINTENDO_SWITCH;
    //             mode_config.applicable_backends_count = 3;
    //             break;
    //         case MODE_MELEE:
    //         case MODE_PROJECT_M:
    //         case MODE_ULTIMATE:
    //         case MODE_RIVALS_OF_AETHER:
    //             mode_config.rgb_config = 1;
    //             mode_config.layout_plate = LAYOUT_PLATE_PLATFORM_FIGHTER;
    //             mode_config.applicable_backends[0] = COMMS_BACKEND_XINPUT;
    //             mode_config.applicable_backends[1] = COMMS_BACKEND_DINPUT;
    //             mode_config.applicable_backends[2] = COMMS_BACKEND_NINTENDO_SWITCH;
    //             mode_config.applicable_backends[3] = COMMS_BACKEND_GAMECUBE;
    //             mode_config.applicable_backends_count = 4;
    //             break;
    //         case MODE_KEYBOARD:
    //             mode_config.applicable_backends[0] = COMMS_BACKEND_DINPUT;
    //             mode_config.applicable_backends_count = 1;
    //             mode_config.rgb_config = 1;
    //             break;
    //         default:
    //             mode_config.layout_plate = LAYOUT_PLATE_EVERYTHING;
    //     }
    // }

    return config;
}

size_t init_secondary_backends_glyph(
    CommunicationBackend **&backends,
    CommunicationBackend *&primary_backend,
    CommunicationBackendId backend_id,
    InputState &inputs,
    InputSource **input_sources,
    size_t input_source_count,
    Config &config,
    const Pinout &pinout
) {
    size_t backend_count = init_secondary_backends_default(
        backends,
        primary_backend,
        backend_id,
        inputs,
        input_sources,
        input_source_count,
        config,
        pinout
    );

    return backend_count;
}

#endif
