#include "display/DefaultConfigMenu.hpp"
#include "display/ConfigMenuAssets/GlyphMenuBitmaps.h"
#include "core/Persistence.hpp"
#include "core/config_utils.hpp"
#include "core/mode_selection.hpp"
#include "reboot.hpp"
#include "config/glyph/common/include/LEDTemplates.hpp"
#include "comms/NeoPixelBackend.hpp"
#include "img/update.hpp"

DefaultConfigMenu::DefaultConfigMenu(
    Config &config,
    CommunicationBackend **backends,
    size_t backends_count
)
    : ConfigMenu(config, backends, backends_count) {
    /* Build default USB backends page */
    MenuPage::MenuItem *usb_backend_options =
        new MenuPage::MenuItem[config.communication_backend_configs_count];

    size_t usb_backend_options_count = 0;
    for (size_t i = 0; i < config.communication_backend_configs_count; i++) {
        CommunicationBackendConfig &backend_config = config.communication_backend_configs[i];
        MenuPage::MenuItem &current_option = usb_backend_options[usb_backend_options_count];

        if(_backends[0]->CurrentGameMode()->GetConfig()->keyboard_mode_config != 0) {
            if(backend_config.backend_id != COMMS_BACKEND_DINPUT) {
                continue;
            }
        } else if (backend_config.backend_id != COMMS_BACKEND_XINPUT &&
            backend_config.backend_id != COMMS_BACKEND_DINPUT &&
            backend_config.backend_id != COMMS_BACKEND_NINTENDO_SWITCH &&
            backend_config.backend_id != COMMS_BACKEND_PASSTHROUGH_PS4 &&
            backend_config.backend_id != COMMS_BACKEND_PASSTHROUGH_PS5) {
            continue;
        }

        strlcpy(
            current_option.text,
            backend_name(backend_config.backend_id),
            sizeof(current_option.text)
        );
        current_option.key = i;
        current_option.action = &SetUsbBackend;
        usb_backend_options_count++;
    }

    _usb_backends_page = {
        .items = usb_backend_options,
        .items_count = usb_backend_options_count,
    };

    /* Build gamemodes page */
    MenuPage::MenuItem *gamemode_options = new MenuPage::MenuItem[config.game_mode_configs_count];

    size_t gamemode_options_count = 0;
    for (size_t i = 0; i < config.game_mode_configs_count; i++) {
        GameModeConfig &mode_config = config.game_mode_configs[i];
        MenuPage::MenuItem &current_option = gamemode_options[gamemode_options_count];

        if (_backends_count > 0 && _backends[0] != nullptr) {
            CommunicationBackendId primary_backend_id = _backends[0]->BackendId();
            if ( !(primary_backend_id == COMMS_BACKEND_DINPUT
                    || primary_backend_id == COMMS_BACKEND_XINPUT
                    || primary_backend_id == COMMS_BACKEND_NINTENDO_SWITCH
                    || primary_backend_id == COMMS_BACKEND_PASSTHROUGH_PS4
                    || primary_backend_id == COMMS_BACKEND_PASSTHROUGH_PS5) &&
                mode_config.mode_id == MODE_KEYBOARD) {
                continue;
            }
        }

        if (strnlen(mode_config.name, sizeof(mode_config.name)) > 0) {
            strlcpy(current_option.text, mode_config.name, sizeof(current_option.text));
        } else {
            strlcpy(
                current_option.text,
                gamemode_name(mode_config.mode_id),
                sizeof(current_option.text)
            );
        }
        current_option.key = i;
        current_option.action = &SetDefaultMode;
        gamemode_options_count++;
    }

    _gamemode_options_page = {
        .items = gamemode_options,
        .items_count = gamemode_options_count,
    };

    /* Build SOCD types page */
    static MenuPage::MenuItem socd_options[_SocdType_MAX] = {};
    for (uint8_t socd_type = SOCD_NEUTRAL; socd_type < _SocdType_ARRAYSIZE; socd_type++) {
        MenuPage::MenuItem &current_option = socd_options[socd_type - 1];
        strlcpy(current_option.text, socd_name((SocdType)socd_type), sizeof(current_option.text));
        current_option.key = socd_type;
        current_option.action = &SetSocdType;
    }

/*
    static MenuPage socd_page = {
        .items = socd_options,
        .items_count = sizeof(socd_options) / sizeof(MenuPage::MenuItem),
    };
*/
    /* Build top-level page */
    // clang-format off
    static MenuPage::MenuItem top_level_items[] = {
        {
            .text = ".Profile",
            .page = &_gamemode_options_page,
            .smallIcon = bitmap_gamemode_small,
            .largeIcon = bitmap_gamemode_large,
            .usb = false,
        },
        {
            .text = ".USB Mode",
            .page = &_usb_backends_page,
            .smallIcon = bitmap_usb_small,
            .largeIcon = bitmap_usb_large,
            .usb = true,
        },
        {
            .text = "RGB.Brightness",
            .action = [](
                IntegratedDisplay *display_backend,
                ConfigMenu *menu,
                Config &config,
                uint8_t key
            ) {
                display_backend->SetDisplayMode(DISPLAY_MODE_RGB_BRIGHTNESS);
            },
            .smallIcon = bitmap_brightness_small,
            .largeIcon = bitmap_brightness_large,
            .usb = false,
        },
        {
            .text = "Input.Viewer",
            .action = [](
                IntegratedDisplay *display_backend,
                ConfigMenu *menu,
                Config &config,
                uint8_t key
            ) {
                // Restore gamemode.
                DefaultConfigMenu *config_menu = (DefaultConfigMenu*)menu; 
                if (config_menu->_backends[0] != nullptr) {
                    config_menu->_backends[0]->SetGameMode(display_backend->CurrentGameMode());
                }
                display_backend->SetDisplayMode(DISPLAY_MODE_VIEWER);
            },
            .smallIcon = bitmap_input_small,
            .largeIcon = bitmap_input_large,
            .usb = false,
        },
        {
            .text = "Connect to.Configurator",
            .action = [](
                IntegratedDisplay *display_backend,
                ConfigMenu *menu,
                Config &config,
                uint8_t key
            ) {
                display_backend->Clear();
                Adafruit_GFX *display = &display_backend->_display;
                display->fillScreen(0);
                display->setCursor(2, 11);
                display->print("Configurator Mode");

                display->setCursor(2, 50);
                display->print("Please visit [URL]");
                display->setCursor(2, 61);
                display->print("to connect");
                //whatever

                for(size_t i = 0; i < config.communication_backend_configs_count; i++) {
                    if(config.communication_backend_configs[i].backend_id == COMMS_BACKEND_CONFIGURATOR) {
                        //TODO: maybe use another byte so I don't have to deal with this
                        //rp2040.idleOtherCore();
                        tud_disconnect();
                        watchdog_hw->scratch[0] = i + 1;
                        display_backend->UpdateDisplay();
                        reboot_firmware();
                    }
                }
            },
            .smallIcon = bitmap_config_small,
            .largeIcon = bitmap_config_large,
            .usb = true,
        },
        {
            .text = "Manual FW.Update",
            .action = [](
                IntegratedDisplay *display_backend,
                ConfigMenu *menu,
                Config &config,
                uint8_t key
            ) {
                display_backend->Clear();
                Adafruit_GFX *display = &display_backend->_display;
                
                // Draw update splash image (128x64 mono)
                display->drawBitmap(0, 0, Bitmap_Update, 128, 64, 1);
                display_backend->UpdateDisplay();

                CRGB *ledArray = FastLED.leds();
                for(int i = 0; i < 76; i++) {
                    ledArray[i] = bootloaderRGB[i];
                }
                FastLED.show();

                reboot_bootloader();
            },
            .smallIcon = bitmap_firmware_small,
            .largeIcon = bitmap_firmware_large,
            .usb = true,
        },
        {
            .text = "Gaming.(coming soon)",
            .action = [](
                IntegratedDisplay *display_backend,
                ConfigMenu *menu,
                Config &config,
                uint8_t key
            ) {
                /*
                display_backend->Clear();
                display_backend->UpdateDisplay();
                display_backend->SetDisplayMode(DISPLAY_MODE_ABOUT);
                */
            },
            .smallIcon = bitmap_gameing_small,
            .largeIcon = bitmap_gameing_large,
            .usb = false,
        },
        {
            .text = "About",
            .action = [](
                IntegratedDisplay *display_backend,
                ConfigMenu *menu,
                Config &config,
                uint8_t key
            ) {
                display_backend->Clear();
                display_backend->UpdateDisplay();
                display_backend->SetDisplayMode(DISPLAY_MODE_ABOUT);
            },

            .smallIcon = bitmap_about_small,
            .largeIcon = bitmap_about_large,
            .usb = false,
        },
    };
    // clang-format on

    static MenuPage top_level_page = {
        .items = top_level_items,
        .items_count = sizeof(top_level_items) / sizeof(MenuPage::MenuItem),
    };
    _top_level_page = &top_level_page;

    _usb_backends_page.parent = _top_level_page;
    _gamemode_options_page.parent = _top_level_page;
    //socd_page.parent = _top_level_page;

    // Set initial page.
    _current_menu_page = _top_level_page;
}

DefaultConfigMenu::~DefaultConfigMenu() {
    delete[] _usb_backends_page.items;
    delete[] _gamemode_options_page.items;
}

void DefaultConfigMenu::SetDefaultMode(
    IntegratedDisplay *display_backend,
    ConfigMenu *menu,
    Config &config,
    uint8_t mode_config_index
) {
    if (mode_config_index < 0 || mode_config_index >= config.game_mode_configs_count) {
        return;
    }

    uint8_t forced_backend_index = 0;
    //dinput is the only valid usb backend for keyboard modes
    if(config.game_mode_configs[mode_config_index].keyboard_mode_config != 0) {
        //find dinput backend
        for(size_t i = 0; i < config.communication_backend_configs_count; i++) {
            if(config.communication_backend_configs[i].backend_id == COMMS_BACKEND_DINPUT) {
                forced_backend_index = i + 1;
                break;
            }
        }
    }

    tud_disconnect();
    //only required in xinput mode...for now
    delay(500);

    watchdog_hw->scratch[0] = forced_backend_index;
    watchdog_hw->scratch[1] = mode_config_index + 1;
    delay(30);
    reboot_firmware();
}

//18 len, as defined in the protobuf
bool sameName(char* name1, char* name2) {
    for(size_t i = 0; i < 18; i++) {
        if(name1[i] != name2[i]) {
            return false;
        }
    }
    return true;
}

void DefaultConfigMenu::SetUsbBackend(
    IntegratedDisplay *display_backend,
    ConfigMenu *menu,
    Config &config,
    uint8_t backend_config_index
) {
    if (backend_config_index < 0 ||
        backend_config_index >= config.communication_backend_configs_count) {
        return;
    }

    char* name = display_backend->CurrentGameMode()->GetConfig()->name;

    for(size_t i = 0; i < config.game_mode_configs_count; i++) {
        if(sameName(name, config.game_mode_configs[i].name)) {
            tud_disconnect();
            delay(500);
            watchdog_hw->scratch[1] = i + 1;
            watchdog_hw->scratch[0] = backend_config_index + 1;
            delay(30);
            reboot_firmware();
        }
    }

}

void DefaultConfigMenu::SetSocdType(
    IntegratedDisplay *display_backend,
    ConfigMenu *menu,
    Config &config,
    uint8_t socd_type
) {
    return;
    /*
    if (socd_type <= SOCD_UNSPECIFIED || socd_type > _SocdType_MAX) {
        return;
    }

    // Overwrite SOCD type for all SOCD pairs of current gamemode's config.
    GameModeConfig *mode_config = display_backend->CurrentGameMode()->GetConfig();
    if (mode_config != nullptr) {
        for (size_t i = 0; i < mode_config->socd_pairs_count; i++) {
            mode_config->socd_pairs[i].socd_type = (SocdType)socd_type;
        }
    }
    */
}
