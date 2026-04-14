#include "display/GlyphConfigMenu.hpp"

#include "core/Persistence.hpp"
#include "core/config_utils.hpp"
#include "core/mode_selection.hpp"
#include "reboot.hpp"

GlyphConfigMenu::GlyphConfigMenu(
    Config &config,
    CommunicationBackend **backends,
    size_t backends_count
)
    : DefaultConfigMenu(config, backends, backends_count) {
    /* Build gamemodes page */
    size_t gamemode_options_count = 0;

    CommunicationBackendId primary_backend_id = _backends[0]->BackendId();
    bool is_in_usb_mode = false;
    switch(primary_backend_id) {
        case COMMS_BACKEND_XINPUT:
        case COMMS_BACKEND_DINPUT:
        case COMMS_BACKEND_NINTENDO_SWITCH:
        case COMMS_BACKEND_PASSTHROUGH_PS4:
        case COMMS_BACKEND_PASSTHROUGH_PS5:
            is_in_usb_mode = true;
            break;
        default: 
            break;
    }
    for (size_t i = 0; i < config.game_mode_configs_count; i++) {
        GameModeConfig &mode_config = config.game_mode_configs[i];
        MenuPage::MenuItem &current_option = _gamemode_options_page.items[gamemode_options_count];

        // Only show gamemodes that are applicable to the current comms backend.
        if (mode_config.applicable_backends_count > 0 && _backends_count > 0 &&
            _backends[0] != nullptr) {
            bool applicable = false;
            for (size_t j = 0; j < mode_config.applicable_backends_count; j++) {
                bool is_keyboard = mode_config.keyboard_mode_config != 0;
                if (primary_backend_id == mode_config.applicable_backends[j]) {
                    applicable = true;
                    break;
                } else if(is_keyboard && is_in_usb_mode) {
                    applicable = true;
                }
            }
            if (!applicable) {
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
    _gamemode_options_page.items_count = gamemode_options_count;

    /* Add new page for dashboard option */
    // clang-format off
    size_t dashboard_options_count = 2;
    static MenuPage::MenuItem dashboard_options[] = {
        {
            .text = "Dashboard Menu",
            .key = DASHBOARD_MENU_BUTTON_HINTS,
            .action = &SetDashboardOption,
        },
        {
            .text = "Input Viewer",
            .key = DASHBOARD_INPUT_VIEWER,
            .action = &SetDashboardOption,
        },
    };
    // clang-format on

    static MenuPage dashboard_options_page = {
        .parent = _top_level_page,
        .items = dashboard_options,
        .items_count = sizeof(dashboard_options) / sizeof(MenuPage::MenuItem),
    };

    /* Overrides for top-level page */
    /*
    strlcpy(_top_level_page->items[2].text, "SOCD Option", sizeof(_top_level_page->items[2].text));
    _top_level_page->items[4] = {
        .text = "Dashboard Option",
        .page = &dashboard_options_page,
    };
    */
}

void GlyphConfigMenu::ReturnToDashboard(IntegratedDisplay *instance) {
    instance->SetDisplayMode(
        _config.default_dashboard_option == DASHBOARD_INPUT_VIEWER ? DISPLAY_MODE_VIEWER
                                                                   : DISPLAY_MODE_BUTTON_HINTS
    );
}

void GlyphConfigMenu::SetDashboardOption(
    IntegratedDisplay *display_backend,
    ConfigMenu *menu,
    Config &config,
    uint8_t dashboard_option
) {
    // Restore gamemode.
    GlyphConfigMenu *config_menu = (GlyphConfigMenu *)menu;
    if (config_menu->_backends[0] != nullptr) {
        config_menu->_backends[0]->SetGameMode(display_backend->CurrentGameMode());
    }

    config.default_dashboard_option = (DashboardOption)dashboard_option;
    config_menu->ReturnToDashboard(display_backend);
}
