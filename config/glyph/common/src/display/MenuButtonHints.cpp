#include "display/MenuButtonHints.hpp"

#include "comms/IntegratedDisplay.hpp"
#include "core/config_utils.hpp"
#include "util/state_util.hpp"

#include "icons/12x12bitmaps.hpp"
#include "icons/16x16bitmaps.hpp"
#include "icons/menubases.hpp"
#include "display/Picopixel.h"
#include "display/Font4x7Fixed.h"
#include <iostream>
#include <string>


const unsigned char *switchIcon(OutputOption output_option) {
    //return a_twelve;
    //need to add: OUT_XB_START, OUT_XB_BACK
    switch (output_option) {
        case OUT_A:
            return a_twelve;
        case OUT_B:
            return b_twelve;
        case OUT_X:
            return x_twelve;
        case OUT_Y:
            return x_twelve;
        case OUT_TRIANGLE:
            return triangle_twelve;
        case OUT_CIRCLE:
            return circle_twelve;
        case OUT_MENU:
            return xb_menu_twelve;
        case OUT_START:
            return Start_12;
        case OUT_SQUARE:
            return square_twelve;
        case OUT_SHARE:
            return sw_share_twelve;
            //return xb_share_twelve;
        case OUT_PLUS:
            return plus_twelve;
        case OUT_MINUS:
            return minus_twelve;
        case OUT_HOME:
            return home_twelve;
        case OUT_TP:
            return touchpad_twelve;
        case OUT_SW_CAPTURE:
            return sw_share_twelve;
        case OUT_R:
            return r_twelve;
        case OUT_L:
            return l_twelve;
        case OUT_ZR:
            return zr_twelve;
        case OUT_ZL:
            return zl_twelve;
        case OUT_RT:
            return rt_twelve;
        case OUT_LT:
            return lt_twelve;
        case OUT_LB:
            return lb_twelve;
        case OUT_RB:
            return rb_twelve;
        case OUT_R3:
            return r3_twelve;
        case OUT_L3:
            return l3_twelve;
        case OUT_Z:
            return z_twelve;
        case OUT_DPAD_DOWN:
            return dpad_down_twelve;
        case OUT_DPAD_UP:
            return dpad_up_twelve;
        case OUT_DPAD_LEFT:
            return dpad_left_twelve;
        case OUT_DPAD_RIGHT:
            return dpad_right_twelve;
        case OUT_XB_START:
            return XB_Start_12;
        case OUT_XB_BACK:
            return XB_Back_12; 
        default:
            return nullptr;
    }
}

//switch mode +usb mode is a last-second hack
//makes sure it only outputs the capture icon if it's actually connected to a switch
const unsigned char *currentInputIcon(OutputState buttons, CommunicationBackendId backend, bool switch_mode, bool usb_connected) {
    if(buttons.a) return Bitmap_A_16;
    if(buttons.b) return Bitmap_B_16;
    if(buttons.x) return Bitmap_X_16;
    if(buttons.y) return Bitmap_Y_16;
    if(buttons.buttonL) {
        if(backend == COMMS_BACKEND_NINTENDO_SWITCH || backend == COMMS_BACKEND_SNES) {
            return Bitmap_L_16;
        }
        return Bitmap_LB_16;
    }
    if(buttons.buttonR) {
        if(backend == COMMS_BACKEND_GAMECUBE) {
            return Bitmap_Z_16;
        }
        if(backend == COMMS_BACKEND_NINTENDO_SWITCH || backend == COMMS_BACKEND_SNES) {
            return Bitmap_R_16;
        }
        return Bitmap_RB_16;
    }
    if(buttons.triggerLDigital) {
        if(backend == COMMS_BACKEND_NINTENDO_SWITCH) {
            return Bitmap_ZL_16;
        }
        if(backend == COMMS_BACKEND_GAMECUBE) {
            return Bitmap_L_16;
        }
        return Bitmap_LT_16;
    }
    if(buttons.triggerRDigital) {
        if(backend == COMMS_BACKEND_NINTENDO_SWITCH) {
            return Bitmap_ZR_16;
        }
        if(backend == COMMS_BACKEND_GAMECUBE) {
            return Bitmap_R_16;
        }
        return Bitmap_RT_16;
    }
    if(buttons.start) {
        if(backend == COMMS_BACKEND_NINTENDO_SWITCH) {
            return Bitmap_Plus_16;
        } else if (backend == COMMS_BACKEND_PASSTHROUGH_PS4 || backend == COMMS_BACKEND_PASSTHROUGH_PS5) {
            return Bitmap_Start_16;
        } else if (backend == COMMS_BACKEND_XINPUT || backend == COMMS_BACKEND_DINPUT) {
            return Bitmap_XB_Start_16;
        }
        if(switch_mode) return Bitmap_Plus_16;
        //if(usb_connected) return Bitmap_XB_Start_16;
        return Bitmap_Start_16;
    }
    if(buttons.select) {
        if(backend == COMMS_BACKEND_NINTENDO_SWITCH) {
            return Bitmap_Minus_16;
        } else if (backend == COMMS_BACKEND_PASSTHROUGH_PS4 || backend == COMMS_BACKEND_PASSTHROUGH_PS5) {
            return Bitmap_SWShare_16;
        } else if (backend == COMMS_BACKEND_XINPUT || backend == COMMS_BACKEND_DINPUT) {
            return Bitmap_XB_Back_16;
        }
        if(switch_mode) {
            return Bitmap_Minus_16;
        }
        if(usb_connected) {
            return Bitmap_XB_Back_16;
        }
    }
    if(buttons.home) {
        if(backend == COMMS_BACKEND_NINTENDO_SWITCH) {
            return Bitmap_Home_16;
        } else if (backend == COMMS_BACKEND_PASSTHROUGH_PS4 || backend == COMMS_BACKEND_PASSTHROUGH_PS5) {
            return Bitmap_Home_16;
        } else if (backend == COMMS_BACKEND_XINPUT || backend == COMMS_BACKEND_DINPUT) {
            return Bitmap_Home_16;
        }
        if(usb_connected) {
            return Bitmap_Home_16;
        }
    }
    if(buttons.capture) {
        if(backend == COMMS_BACKEND_NINTENDO_SWITCH) {
            return Bitmap_SWShare_16;
        }
        if(backend == COMMS_BACKEND_PASSTHROUGH_PS4 || backend == COMMS_BACKEND_PASSTHROUGH_PS5) {
            return Bitmap_Touchpad_16;
        }
        if(switch_mode) {
            return Bitmap_SWShare_16;
        }
        //return Bitmap_XBShare_16;
    }
    if(buttons.dpadDown) return Bitmap_DPadDown_16;
    if(buttons.dpadUp) return Bitmap_DpadUp_16;
    if(buttons.dpadLeft) return Bitmap_DPadLeft_16;
    if(buttons.dpadRight) return Bitmap_DpadRight_16;
    if(buttons.leftStickClick) return Bitmap_LSClick_16;
    if(buttons.rightStickClick) return Bitmap_RSClick_16;

    if(buttons.modX) return Bitmap_ModX_16;
    if(buttons.modY) return Bitmap_ModY_16;

    if(buttons.leftStickLeft) return Bitmap_LS_Left_16;
    if(buttons.leftStickRight) return Bitmap_LS_Right_16;
    if(buttons.leftStickUp) return Bitmap_LS_Up_16;
    if(buttons.leftStickDown) return Bitmap_LS_Down_16;

    if(buttons.rightStickLeft) {
        if(backend == COMMS_BACKEND_GAMECUBE) return Bitmap_CS_Left_16;
        return Bitmap_RS_Left_16;
    }
    if(buttons.rightStickRight) {
        if(backend == COMMS_BACKEND_GAMECUBE) return Bitmap_CS_Right_16;
        return Bitmap_RS_Right_16;
    }
    if(buttons.rightStickUp) {
        if(backend == COMMS_BACKEND_GAMECUBE) return Bitmap_CS_Up_16;
        return Bitmap_RS_Up_16;
    }
    if(buttons.rightStickDown) {
        if(backend == COMMS_BACKEND_GAMECUBE) return Bitmap_CS_Down_16;
        return Bitmap_RS_Down_16;
    }

    return nullptr;
}


MenuButtonHints::MenuButtonHints(CommunicationBackend **backends, size_t backends_count)
    : _backends(backends),
      _backends_count(backends_count) {
    _backend_id = backends[0]->BackendId();
}

DisplayModeId MenuButtonHints::GetId() {
    return DISPLAY_MODE_BUTTON_HINTS;
}

void MenuButtonHints::HandleControls(
    IntegratedDisplay *instance,
    const DisplayControls &controls,
    Button button
) {
    if (button == controls.back) {
        instance->SetDisplayMode(DISPLAY_MODE_CONFIG);
    }
}


constexpr const char *plate_name(LayoutPlate plate) {
    switch(plate) {
        case LAYOUT_PLATE_UNSPECIFIED: 
            return "Unknown Plate";
        case LAYOUT_PLATE_EVERYTHING: 
            return "Full";
        case LAYOUT_PLATE_FGC:
            return "FGC";
        case LAYOUT_PLATE_SPLIT_FGC:
            return "Split FGC";
        case LAYOUT_PLATE_PLATFORM_FIGHTER:
            return "Plat Fighter";
    }
    return "Unknown Plate";
}

constexpr const char *get_stick_string(uint8_t center, uint8_t value) {
    if(value > center) {

    }
    return ":p";
}

void MenuButtonHints::UpdateDisplay(IntegratedDisplay *instance, Adafruit_GFX &display) {
    InputState &inputs = instance->GetInputs();
    uint8_t font_width = instance->font_width;
    uint8_t color = instance->default_color;
    OutputState &outputs = _backends[0]->GetOutputs();
    uint32_t buttons = _backends[0]->GetOutputs().buttons;
    
    display.drawBitmap(0, 0, Bitmap_Dashboard_Base_V3, 128, 64, 1);

    UpdateButtonHints(instance);

    /* Gamemode text */
    //display.setCursor(0, 0);
    display.setFont(&Font4x7Fixed);
    display.setTextSize(1); 
    

    if (instance->CurrentGameMode() != nullptr) {
        const GameModeConfig &mode_config = *instance->CurrentGameMode()->GetConfig();
        /* profile info */
        display.setCursor(12, 20);
        if (strnlen(mode_config.name, sizeof(mode_config.name)) > 0) {
            display.print(mode_config.name);
        } else {
            display.print(gamemode_name(mode_config.mode_id));
        }

        display.setCursor(12, 30);
        display.print(plate_name(mode_config.layout_plate));

        const char *backend_text = backend_name(_backend_id);
        display.setCursor(12, 40);
        display.print(backend_name(_backend_id));    

        /* Currently activated output */ 
        CommunicationBackendId iconBackend = COMMS_BACKEND_UNSPECIFIED;
        
        //if there's a console mode + usb, use the console icons where possible
        for(size_t i = 0; i < mode_config.applicable_backends_count; i++) {
            CommunicationBackendId b = mode_config.applicable_backends[i];
            if(b == COMMS_BACKEND_UNSPECIFIED) continue; //should never happen lol
            if(b == COMMS_BACKEND_DINPUT || b == COMMS_BACKEND_NINTENDO_SWITCH || b == COMMS_BACKEND_XINPUT ||
               b == COMMS_BACKEND_PASSTHROUGH_PS4 || b == COMMS_BACKEND_PASSTHROUGH_PS5) {
                //need to change this based on the current usb backend but this is fine for now
                //probably don't need to do anything here rn
            } else {
                iconBackend = b;
            } 
        }
        bool connected_to_usb = false;
        switch (_backend_id) {
            case COMMS_BACKEND_DINPUT:
            case COMMS_BACKEND_XINPUT:
            case COMMS_BACKEND_NINTENDO_SWITCH:
            case COMMS_BACKEND_PASSTHROUGH_PS4:
            case COMMS_BACKEND_PASSTHROUGH_PS5:
                connected_to_usb = true;
                break;    
            default:
                break;
        }

        if(iconBackend == COMMS_BACKEND_UNSPECIFIED) iconBackend = _backends[0]->BackendId();
        const unsigned char* currentInput = currentInputIcon(outputs, iconBackend, _backend_id == COMMS_BACKEND_NINTENDO_SWITCH ? true : false, connected_to_usb);
        if(currentInput != nullptr) {
            display.drawBitmap(109, 3, currentInput, 16, 16, 1);
        }
    }

    display.setCursor(12, 10);
    display.print("Glyph");

    //mb2 
    if(inputs.mb2) {
        display.fillRect(20, 46, 18, 18, 1);
    }
    if(mb2_bmp != nullptr) display.drawBitmap(22, 48, mb2_bmp, 12, 12, !inputs.mb2 ); 
    //mb3
    if(inputs.mb3) {
        display.fillRect(38, 46, 18, 18, 1);
    }
    if(mb3_bmp != nullptr) display.drawBitmap(40, 48, mb3_bmp, 12, 12, !inputs.mb3 );
    //mb4
    if(inputs.mb4) {
        display.fillRect(56, 46, 18, 18, 1);
    }
    if(mb4_bmp != nullptr) display.drawBitmap(58, 48, mb4_bmp, 12, 12, !inputs.mb4 );
    //mb5
    //I think the icon is one-off
    if(inputs.mb5) {
        display.fillRect(74, 46, 18, 18, 1);
    }
    if(mb5_bmp != nullptr) display.drawBitmap(76, 48, mb5_bmp, 12, 12, !inputs.mb5);
    //mb6
    if(inputs.mb6) {
        display.fillRect(92, 46, 18, 18, 1);
    }
    if(mb6_bmp != nullptr) display.drawBitmap(94, 48, mb6_bmp, 12, 12, !inputs.mb6);
    //mb7
    if(inputs.mb7) {
        display.fillRect(110, 46, 18, 18, 1);
    }
    if(mb7_bmp != nullptr) display.drawBitmap(112, 48, mb7_bmp, 12, 12, !inputs.mb7);
    

    display.setFont(&Picopixel);

    //X
    display.setCursor(78, 10);
    display.print((int)(outputs.leftStickX - 128));

    display.setCursor(78, 18);
    display.print((int)(outputs.leftStickY - 128));

    display.setCursor(84, 26);
    display.print((int)(outputs.rightStickX - 128));

    display.setCursor(84, 34);
    display.print((int)(outputs.rightStickY - 128));

    display.setCursor(112, 26);
    display.print((int)(outputs.triggerRAnalog));

    display.setCursor(112, 34);
    display.print((int)(outputs.triggerLAnalog));

    //display.setCursor(0, 20);
    //const char *currently_pressed_button_text = CurrentlyPressedButtonText();
    // Lazy debounce on the output state, because it may be updated from the other core which could
    // cause flickering. This debounce makes it so the button text has to change for 5 consecutive
    // milliseconds before it will update on the display.
    /*
    if (strcmp(currently_pressed_button_text, _last_pressed_button_text) == 0) {
        _pressed_output_locked_until = 0;
    } else {
        if (_pressed_output_locked_until == 0) {
            _pressed_output_locked_until = make_timeout_time_ms(5);
        } else if (time_reached(_pressed_output_locked_until)) {
            _last_pressed_button_text = currently_pressed_button_text;
        }
    }
    display.print(_last_pressed_button_text);
    display.setCursor(0, 0);
    */
}

Button findMenuBind(Button menu_button, GameModeConfig mode_config) {   
    for(int i = 0; i < mode_config.button_remapping_count; i++) {
        ButtonRemap remap = mode_config.button_remapping[i];
        if(remap.physical_button == menu_button) {
            return remap.activates;
        }
    }
    return BTN_UNSPECIFIED;
}

void MenuButtonHints::UpdateButtonHints(IntegratedDisplay *instance) {
    //need to set up icons
    if (instance->CurrentGameMode() != nullptr) { 
        const GameModeConfig &mode_config = *instance->CurrentGameMode()->GetConfig();

        mb1_bmp = switchIcon(mode_config.menu_button_icon[0]);
        mb2_bmp = switchIcon(mode_config.menu_button_icon[1]);
        mb3_bmp = switchIcon(mode_config.menu_button_icon[2]);
        mb4_bmp = switchIcon(mode_config.menu_button_icon[3]);
        mb5_bmp = switchIcon(mode_config.menu_button_icon[4]);
        mb6_bmp = switchIcon(mode_config.menu_button_icon[5]);
        mb7_bmp = switchIcon(mode_config.menu_button_icon[6]);
    }
    
}

const char *MenuButtonHints::CurrentlyPressedButtonText() {
    OutputState &outputs = _backends[0]->GetOutputs();
    uint32_t buttons = _backends[0]->GetOutputs().buttons;

    for (uint8_t i = 0; i < _DigitalOutput_MAX; i++) {
        bool activated = buttons & (1UL << i);
        if (activated) {
            return digital_output_name((DigitalOutput)(i + 1));
        }
    }
    if (outputs.leftStickX < (128 - 5)) {
        return "Left Stick X-";
    } else if (outputs.leftStickX > (128 + 5)) {
        return "Left Stick X+";
    } else if (outputs.leftStickY < (128 - 5)) {
        return "Left Stick Y-";
    } else if (outputs.leftStickY > (128 + 5)) {
        return "Left Stick Y+";
    } else if (outputs.leftStickX < (128 - 5)) {
        return "Right Stick X-";
    } else if (outputs.leftStickX > (128 + 5)) {
        return "Right Stick X+";
    } else if (outputs.leftStickY < (128 - 5)) {
        return "Right Stick Y-";
    } else if (outputs.leftStickY > (128 + 5)) {
        return "Right Stick Y+";
    } else if (outputs.triggerLAnalog > 0) {
        return "L2 Analog";
    } else if (outputs.triggerRAnalog > 0) {
        return "R2 Analog";
    }
    return "";
}
