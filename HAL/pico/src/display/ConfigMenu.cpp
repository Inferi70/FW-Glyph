#include "display/ConfigMenu.hpp"
#include "display/ConfigMenuAssets/GlyphMenuBitmaps.h"
#include "config/glyph/common/include/display/Font4x7Fixed.h"
#include "config/glyph/common/include/icons/12x12bitmaps.hpp"

#include "core/Persistence.hpp"
#include "core/config_utils.hpp"
#include "core/mode_selection.hpp"
#include "reboot.hpp"

ConfigMenu::ConfigMenu(Config &config, CommunicationBackend **backends, size_t backends_count)
    : _config(config),
      _backends(backends),
      _backends_count(backends_count) {}

DisplayModeId ConfigMenu::GetId() {
    return DISPLAY_MODE_CONFIG;
}

void ConfigMenu::HandleControls(
    IntegratedDisplay *instance,
    const DisplayControls &controls,
    Button button
) {
    if (_current_menu_page == nullptr) {
        _current_menu_page = _top_level_page;
        _current_menu_offset = 0;
        return;
    }

    if (button == controls.up) {
        if(_current_menu_page == _top_level_page) {
            switch(_backends[0]->BackendId()) {
                case COMMS_BACKEND_DINPUT:
                case COMMS_BACKEND_XINPUT:
                case COMMS_BACKEND_NINTENDO_SWITCH:
                case COMMS_BACKEND_PASSTHROUGH_PS4:
                case COMMS_BACKEND_PASSTHROUGH_PS5:
                    _highlighted_menu_item = max(0, _highlighted_menu_item - 1);
                    break;
                default:
                    _highlighted_menu_item = max(0, _highlighted_menu_item - 1);
                    while(_current_menu_page->items[_highlighted_menu_item].usb == true) {
                        _highlighted_menu_item = max(0, _highlighted_menu_item - 1);
                    }
                    break;
            }

        } else {
            _highlighted_menu_item = max(0, _highlighted_menu_item - 1);
        }
    } else if (button == controls.down) {
        if(_current_menu_page == _top_level_page) {
            switch(_backends[0]->BackendId()) {
                case COMMS_BACKEND_DINPUT:
                case COMMS_BACKEND_XINPUT:
                case COMMS_BACKEND_NINTENDO_SWITCH:
                case COMMS_BACKEND_PASSTHROUGH_PS4:
                case COMMS_BACKEND_PASSTHROUGH_PS5:
                    _highlighted_menu_item =
                        min(_current_menu_page->items_count - 1, _highlighted_menu_item + 1);
                    break;
                default:
                    _highlighted_menu_item =
                        min(_current_menu_page->items_count - 1, _highlighted_menu_item + 1);
                    while(_current_menu_page->items[_highlighted_menu_item].usb == true) {
                        _highlighted_menu_item =
                            min(_current_menu_page->items_count - 1, _highlighted_menu_item + 1);
                    }
                    break;
            }
        } else {
            _highlighted_menu_item =
                min(_current_menu_page->items_count - 1, _highlighted_menu_item + 1);
        }
    } else if (button == controls.enter) {
        // Bounds check.
        if (_highlighted_menu_item > _current_menu_page->items_count) {
            _highlighted_menu_item = 0;
            return;
        }

        const MenuPage::MenuItem &selected_item = _current_menu_page->items[_highlighted_menu_item];

        // If there's an action defined, perform it.
        if (selected_item.action != nullptr) {
            selected_item.action(instance, this, _config, selected_item.key);

            // Go back up a level.
            if (_current_menu_page->parent != nullptr) {
                _current_menu_page = _current_menu_page->parent;
                _current_menu_offset = 0;
            }
            _highlighted_menu_item = 0;
            return;
        }

        // If there's a child page defined, drill into it.
        if (selected_item.page != nullptr) {
            _current_menu_page = selected_item.page;
            _current_menu_offset = 0;
            _highlighted_menu_item = 0;
            return;
        }
    } else if (button == controls.back) {
        // If at top-level page, go back to input viewer.
        if (_current_menu_page->parent == nullptr) {
            // Restore gamemode.
            if (_backends[0] != nullptr) {
                _backends[0]->SetGameMode(instance->CurrentGameMode());
            }
            ReturnToDashboard(instance);
            return;
        }

        _current_menu_page = _current_menu_page->parent;
        _highlighted_menu_item = 0;
    }
}

void ConfigMenu::UpdateDisplay(IntegratedDisplay *instance, Adafruit_GFX &display) {
    // Unset gamemode to prevent menu button presses being sent to console.
    if (_backends[0] != nullptr && _backends[0]->CurrentGameMode() != nullptr) {
        instance->SetGameMode(_backends[0]->CurrentGameMode());
        _backends[0]->SetGameMode(nullptr);
    }

    InputState &inputs = instance->GetInputs();

    display.setFont(&Font4x7Fixed);
    display.setTextWrap(false);

    if(_current_menu_page == _top_level_page) {

        // As far as I can tell, this variable is only used for scrolling
        // the screen on the profile selection menu. Resetting it when the
        // main menu is loaded is needed otherwise it will crash when going
        // back to the main menu after scrolling the screen.
        _current_menu_offset = 0;

        display.drawBitmap(0, 0, bitmap_glyph_menu_base, 128, 64, 1);
        uint8_t margin = 2;
        bool usb = false;
        //usbmargin 17
        switch(_backends[0]->BackendId()) {
            case COMMS_BACKEND_DINPUT:
            case COMMS_BACKEND_XINPUT:
            case COMMS_BACKEND_NINTENDO_SWITCH:
            case COMMS_BACKEND_PASSTHROUGH_PS4:
            case COMMS_BACKEND_PASSTHROUGH_PS5:
                usb = true;
                break;
            default:
                margin = 17;
                break;
        }
        uint8_t spacing = 2;
        uint8_t small_icon_size = 10;
        uint8_t large_icon_size = 28;
        uint8_t y_location = 8;

        uint8_t xVal = 0;
        xVal += margin;
        size_t drawn_item_index = 0;
        for (size_t i = 0; i < _current_menu_page->items_count; i++) {
            if(_current_menu_page->items[i + _current_menu_offset].usb && !usb){
                continue;
            }
            if (i + _current_menu_offset == _highlighted_menu_item) {
                display.drawBitmap(xVal, y_location, _current_menu_page->items[i + _current_menu_offset].largeIcon, 28, 28, 1);
                xVal += spacing + large_icon_size;
                display.setCursor(77, 54);
                for(size_t j = 0; j < 32; j++) {
                    if(_current_menu_page->items[i + _current_menu_offset].text[j] == '.') {
                        display.setCursor(77, 64);
                    } else {
                        display.print(_current_menu_page->items[i + _current_menu_offset].text[j]);
                    }
                }
            } else {
                display.drawBitmap(xVal, y_location, _current_menu_page->items[i + _current_menu_offset].smallIcon, 10, 10, 1);
                xVal += spacing + small_icon_size;
            }
        }

    } else {
        uint8_t font_width = instance->font_width;
        uint8_t font_height = instance->font_height;
        uint8_t xOffset = 78;

        display.drawBitmap(0, 0, bitmap_glyph_list_menu_base, 128, 64, 1);

        display.setCursor(17, 24);
        display.println("Set Profile");

        if (_highlighted_menu_item - _current_menu_offset > max_visible_lines - 1) {
            _current_menu_offset++;
        } else if (_highlighted_menu_item < _current_menu_offset) {
            _current_menu_offset--;
        }
        uint8_t last_item_to_display =
            min(_current_menu_page->items_count - _current_menu_offset, max_visible_lines + 1);
        for (size_t i = 0; i < last_item_to_display; i++) {
            bool highlighted = i + _current_menu_offset == _highlighted_menu_item;

            if (highlighted) {
                display.setCursor(xOffset, (i * (font_height + padding)) + 10);
                display.print(highlight_string);
            }
            display.setCursor(xOffset + font_width + padding, (i * (font_height + padding)) + 10);
            display.print(_current_menu_page->items[i + _current_menu_offset].text);
            if (highlighted) {
                display.print(" <");
            }
        }
    }

    if(inputs.mb1) {
        display.fillRect(2, 46, 18, 18, 1);
        display.drawBitmap(4, 48, Back12, 12, 12, 0);
    }
    if(inputs.mb2) {
        display.fillRect(20, 46, 18, 18, 1);
        if(_current_menu_page == _top_level_page) {
            display.drawBitmap(22, 48, LeftArrow12, 12, 12, 0);
        } else {
            display.drawBitmap(22, 48, UpArrow12, 12, 12, 0);
        }
    }
    if(inputs.mb3) {
        display.fillRect(38, 46, 18, 18, 1);
        if(_current_menu_page == _top_level_page) {
            display.drawBitmap(40, 48, RightArrow12, 12, 12, 0);
        } else {
            display.drawBitmap(40, 48, DownArrow12, 12, 12, 0);
        }
    }
    if(inputs.mb4) {
        display.fillRect(56, 46, 18, 18, 1);
        if(_current_menu_page) {
            display.drawBitmap(58, 48, Confirm12, 12, 12, 0);
        } else {
            display.drawBitmap(58, 48, Confirm12, 12, 12, 0);
        }
    }

/*
    if(inputs.mb1) {
        display.fillRect(2, 46, 18, 18, 1);
    }
*/
}

void ConfigMenu::ReturnToDashboard(IntegratedDisplay *instance) {
    instance->SetDisplayMode(DISPLAY_MODE_VIEWER);
}
