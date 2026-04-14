#include "button_positions.hpp"
#include "comms/backend_init.hpp"
#include "core/CommunicationBackend.hpp"
#include "core/KeyboardMode.hpp"
#include "core/Persistence.hpp"
#include "core/mode_selection.hpp"
#include "core/state.hpp"
#include "display/DisplayMode.hpp"
#include "display/GlyphConfigMenu.hpp"
#include "display/InputDisplay.hpp"
#include "display/MenuButtonHints.hpp"
#include "display/RgbBrightnessMenu.hpp"
#include "display/RemapMenu.hpp"
#include "display/AboutMenu.hpp"
#include "display/OopsieMenu.hpp"
#include "display/Font4x7Fixed.h"
#include "icons/splashscreen.hpp"
#include "glyph_overrides.hpp"
#include "glyph_pinout.hpp"
#include "matrix_definition.hpp"
#include "reboot.hpp"
#include "stdlib.hpp"
#include "LEDTemplates.hpp"
#include "img/update.hpp"
#include "input/USBHostGamepadInput.hpp"
#include "usb/PlayStationAuthPassthrough.hpp"
#include "usb/USBHostAuthPassthrough.hpp"
#include "usb/TinyUSBHostManager.hpp"
#include "usb/USBHostAuthListener.hpp"
#include "usb/XboxAuthPassthrough.hpp"

#include <Adafruit_SSD1306.h>
#include <config.pb.h>
#include "hardware/sync.h"
#include "pico/lock_core.h"

extern bool LED_OK;
extern bool SCREEN_OK;

Config config = glyph_default_config();

DebouncedSwitchMatrixInput<num_rows, num_cols> matrix_input(
    row_pins,
    col_pins,
    matrix,
    DiodeDirection::COL2ROW
);

USBHostGamepadInput usb_host_gamepad_input;
USBHostAuthListener usb_host_auth_listener;

CommunicationBackend **backends = nullptr;
size_t backend_count;
KeyboardMode *current_kb_mode = nullptr;

InputState inputs;

InputSource *input_sources[] = { &matrix_input, &usb_host_gamepad_input };
size_t input_source_count = sizeof(input_sources) / sizeof(InputSource *);

Adafruit_SSD1306 display(128, 64, &OLED_WIRE_INSTANCE);

bool failed_detection = false;
bool main_setup_done = false;


void setup() {
    // Create GPIO input source and use it to read button states for checking button holds.
    matrix_input.UpdateInputs(inputs);

    //start screen early
    Wire1.setSDA(OLED_SDA);
    Wire1.setSCL(OLED_SCL);
    Wire1.setClock(1'000'000UL);
    Wire1.begin();

    if (display.begin(SSD1306_SWITCHCAPVCC, 0x3C, false, false)) {
        display.clearDisplay();
    }

    // Check bootsel button hold as early as possible for safety.
    if (inputs.mb1) {
        // Show update splash image
        display.drawBitmap(0, 0, Bitmap_Update, 128, 64, 1);
        display.display();

        FastLED.addLeds<NEOPIXEL, LED_PIN>(bootloaderRGB, LED_COUNT);
        FastLED.setMaxPowerInVoltsAndMilliamps(5, 200);
        FastLED.setMaxRefreshRate(0);
        FastLED.setBrightness(255);
        FastLED.show();

        reboot_bootloader();
    } else {
        display.drawBitmap(0, 0, Bitmap_Glyph_Splashscreen, 128, 64, 1);
        display.display();
    }

    // Attempt to load config, or write default config to flash if failed to load config.
    if (!persistence.LoadConfig(config)) {
        persistence.SaveConfig(config);
    }

    // Create array of input sources to be used.
    backend_count = initialize_backends(
        backends,
        inputs,
        input_sources,
        input_source_count,
        config,
        pinout,
        get_backend_config_default,
        get_usb_backend_config_default,
        &detect_console,
        &init_secondary_backends_glyph
    );

    setup_mode_activation_bindings(config.game_mode_configs, config.game_mode_configs_count);

    TinyUSBHostManager::instance().pushListener(&usb_host_gamepad_input);
    TinyUSBHostManager::instance().pushListener(&usb_host_auth_listener);
    USBHostAuthPassthrough::instance().attach(&usb_host_auth_listener);
    XboxAuthPassthrough::instance().attach(&usb_host_auth_listener);
    TinyUSBHostManager::instance().start();

    if(backend_count == 0) {
        failed_detection = true;
        backend_count = 1;
    } 

    main_setup_done = true;
}

void loop() {
    if(backends[0] == nullptr) return;
    select_mode(backends, backend_count, config);

    for (size_t i = 0; i < backend_count; i++) {
        backends[i]->SendReport();
    }

    TinyUSBHostManager::instance().process();
    USBHostAuthPassthrough::instance().process();
    PlayStationAuthPassthrough::instance().process();
    XboxAuthPassthrough::instance().process();

    if (current_kb_mode != nullptr) {
        current_kb_mode->SendReport(backends[0]->GetInputs());
    }
}

/* Second core handles OLED display */
IntegratedDisplay *display_backend = nullptr;

RgbBrightnessMenu rgb_brightness_menu(config);

InputDisplay *input_viewer = nullptr;

AboutMenu about_menu(config);

RemapMenu remap_menu;

/* Second core also handles RGB */
NeoPixelBackend<LED_PIN, LED_COUNT> *led_backend = nullptr;

void setup1() {
    while (!main_setup_done) {
        delay(1);
    }
    // These have to be initialized after backends.

    /** rgb zone **/
    led_backend = new NeoPixelBackend<LED_PIN, LED_COUNT>(
        inputs,
        input_sources,
        input_source_count,
        pixel_to_button_mappings,
        config.rgb_configs,
        config.rgb_configs_count,
        config.rgb_brightness
    );

    /** screen zone **/
    CommunicationBackendId primary_backend_id = backends[0]->BackendId();
    static MenuButtonHints menu_button_hints(backends, backend_count);
    static InputDisplay input_display(
        platform_fighter_buttons,
        platform_fighter_buttons_count,
        primary_backend_id
    );
    static GlyphConfigMenu config_menu(config, backends, backend_count);

    static OopsieMenu oopsie_menu(config, backends, backend_count);

    static DisplayMode *display_modes[] = {
        &menu_button_hints,
        &input_display,
        &config_menu,
        &rgb_brightness_menu,
        &about_menu,
        &oopsie_menu,
        &remap_menu
    };
    size_t display_modes_count = count_of(display_modes);

    input_viewer = &input_display;

    //TODO: don't run if display fails init
    // clang-format off
    display_backend = new IntegratedDisplay(
        inputs,
        display,
        []() { display.clearDisplay(); },
        []() { display.display(); },
        DisplayControls{ .back = BTN_MB1, .down = BTN_MB3, .up = BTN_MB2, .enter = BTN_MB4 },
        display_modes,
        display_modes_count
    );
    // clang-format on
    if(failed_detection) {
        display_backend->SetDisplayMode(DISPLAY_MODE_OOPSIE);
        return;
    }


    if(backends[0]->BackendId() == COMMS_BACKEND_CONFIGURATOR) {
        display_backend->SetDisplayMode(DISPLAY_MODE_REMAPPER);
    } else {
        display_backend->SetDisplayMode(
            config.default_dashboard_option == DASHBOARD_INPUT_VIEWER ? DISPLAY_MODE_VIEWER
                                                                    : DISPLAY_MODE_BUTTON_HINTS
        );
    }
    if(display_backend->CurrentDisplayMode() == DISPLAY_MODE_REMAPPER) {
        display_backend->SendReport();
        FastLED.setBrightness(0);
        FastLED.show();
    }
}

void dummyloop() {
    display_backend->SendReport();
}

void loop1() {
    if (display_backend == nullptr) {
        return;
    }

    if(failed_detection) {
        if(!watchdog_caused_reboot()) {
            FastLED.setBrightness(0);
            FastLED.show();
            dummyloop();
            return;
        }
    }

    if(display_backend->CurrentDisplayMode() == DISPLAY_MODE_REMAPPER) {
        //dummyloop();
        while(1);
        return;
    }

    if (backends[0] != nullptr && backends[0]->CurrentGameMode() != nullptr &&
        display_backend->CurrentGameMode() != backends[0]->CurrentGameMode()) {
        display_backend->SetGameMode(backends[0]->CurrentGameMode());
        led_backend->SetGameMode(backends[0]->CurrentGameMode());
    }

    /* 
        This is a hack that only exists for NES mode at the moment 
        It's very, very sensitive about what we do on the other core
        I don't like it but it works.
    */
    bool is_nes = (backends[0] != nullptr && backends[0]->BackendId() == COMMS_BACKEND_NES);

    // Update input display layout.
    if (display_backend->CurrentGameMode() != nullptr) {
        if(!is_nes || SCREEN_OK) {
            GameModeConfig *mode_config = display_backend->CurrentGameMode()->GetConfig();
            switch (mode_config->layout_plate) {
                case LAYOUT_PLATE_UNSPECIFIED:
                case LAYOUT_PLATE_EVERYTHING:
                    input_viewer->UpdateButtonLayout(full_layout_buttons, full_layout_buttons_count);
                    break;
                case LAYOUT_PLATE_FGC:
                    input_viewer->UpdateButtonLayout(fgc_buttons, fgc_buttons_count);
                    break;
                case LAYOUT_PLATE_SPLIT_FGC:
                    input_viewer->UpdateButtonLayout(split_fgc_buttons, split_fgc_buttons_count);
                    break;
                case LAYOUT_PLATE_PLATFORM_FIGHTER:
                    input_viewer->UpdateButtonLayout(
                        platform_fighter_buttons,
                        platform_fighter_buttons_count
                    );
                    break;
            }
        } 
        if(!is_nes) {
            display_backend->SendReport();
        } else if(SCREEN_OK) {
            display_backend->SendReport();
            SCREEN_OK = false;
        }
    }

    if(led_backend != nullptr) {
        if(!is_nes) {
            led_backend->SendReport();
        } else if(LED_OK) {
            led_backend->SendReport();
            LED_OK = false;
        }
    }

}
