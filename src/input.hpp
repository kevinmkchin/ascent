#pragma once

#include "common.hpp"

/** Input manager for SDL

    Examples:
        I want to check if the W key is pressed:
            if(Input::IsKeyPressed(SDL_SCANCODE_W)) ...
                or
            if(Input::gCurrentKeyState[SDL_SCANCODE_W]) ...

        I want to check if the space bar was pressed this frame/step:
            if(Input::HasKeyBeenPressed(SDL_SCANCODE_SPACE)) ...
                or
            if(Input::gJustPressedKeyState[SDL_SCANCODE_SPACE]) ...

        I want to check if Player 1 on gamepad is pressing X button:
            if(Input::GetGamepad(0).IsPressed(GAMEPAD_X)) ...
*/

/** Gamepad **/
#define MAX_GAMEPAD_COUNT   4
#define GAMEPAD_A           0x0001  // bit 0
#define GAMEPAD_B           0x0002  // bit 1
#define GAMEPAD_X           0x0004  // bit 2
#define GAMEPAD_Y           0x0008  // bit 3
#define GAMEPAD_LB          0x0010  // bit 4
#define GAMEPAD_RB          0x0020  // bit 5
#define GAMEPAD_BACK        0x0040  // bit 6
#define GAMEPAD_START       0x0080  // bit 7
#define GAMEPAD_LTHUMB      0x0100  // bit 8
#define GAMEPAD_RTHUMB      0x0200  // bit 9
#define GAMEPAD_DPAD_UP     0x1000  // bit 12
#define GAMEPAD_DPAD_DOWN   0x2000  // bit 13
#define GAMEPAD_DPAD_LEFT   0x4000  // bit 14
#define GAMEPAD_DPAD_RIGHT  0x8000  // bit 15

namespace Input {

//////////////////////////////////////////////////////////////////////
// ASCENT GAME SPECIFIC FUNCTIONS
//////////////////////////////////////////////////////////////////////

    // WHETHER WE ARE PRESSING UP KEY WHATEVER THAT MAY BE
    bool GameUpHasBeenPressed();
    bool GameDownHasBeenPressed();
    bool GameLeftHasBeenPressed();
    bool GameRightHasBeenPressed();
    bool GameUpIsPressed();
    bool GameDownIsPressed();
    bool GameLeftIsPressed();
    bool GameRightIsPressed();
    bool GameJumpHasBeenPressed();
    bool GameJumpHasBeenReleased();
    bool GameJumpIsPressed();
    bool GamePickUpIsPressed();
    bool GameDropIsPressed();
    bool GameThrowIsPressed();

//////////////////////////////////////////////////////////////////////
// GENERIC CLASSES AND FUNCTIONS BELOW
//////////////////////////////////////////////////////////////////////

    struct GamepadState {
        /* Stores the state of an Xbox 360 style controller */
        bool16 isConnected = 0;
        u16 currentButtonState = 0; // check button state using AND (e.g. buttonState & GAMEPAD_A)
        u16 justPressedButtonState = 0;
        u16 justReleasedButtonState = 0;
        float leftTrigger = 0.f; // 0.0 to 1.0
        float rightTrigger = 0.f; // 0.0 to 1.0
        vec2 leftThumbStickDir = {0.f, 0.f}; // normalized vector representing offset
        vec2 rightThumbStickDir = {0.f, 0.f}; // normalized vector representing offset

        bool IsPressed(u16 button /* bitwise-OR-able */) const;

        bool IsReleased(u16 button /* bitwise-OR-able */) const;

        bool HasBeenPressed(u16 button) const;

        bool HasBeenReleased(u16 button) const;

        bool LeftThumbStickUp() const;

        bool LeftThumbStickDown() const;
    };

    struct GamepadStatesWrapper {
        /** InstanceID is an id of an SDL gamepad instance - this increments
            during runtime if a controller is disconnected then reconnected.
            Wrapper is necessary because SDL_JoystickInstanceID doesn't
            necessarily match the gamepad index. This wrapper also
            shifts gamepad indices down when a controller disconnects.
            e.g. if player 1 disconnects, player 2 becomes player 1. */

        u8 mapInstanceIDtoGamepadIndex[128];
        GamepadState states[MAX_GAMEPAD_COUNT];
        u8 gamepadCount = 0;

        // USE THIS OPERATOR TO GET GAMEPAD STATES - 0 = player 1, 1 = player 2, etc.
        GamepadState &operator[](const i32 gamepadIndex) {
            return states[gamepadIndex];
        }

        // IGNORE THESE
        GamepadState &AtInstanceID(const i32 instanceID);

        u8 GamepadIndexFromInstanceID(const i32 instanceID);

        void OnConnect(i32 instanceID);

        void OnDisconnect(i32 instanceID);
    };

/** Keyboard **/
    bool IsKeyPressed(SDL_Scancode scancode);
    bool IsKeyReleased(SDL_Scancode scancode);
    bool HasKeyBeenPressed(SDL_Scancode scancode);
    bool HasKeyBeenReleased(SDL_Scancode scancode);

    void ProcessSDLKeyDownEvent(SDL_KeyboardEvent keyEvent);
    void ProcessSDLKeyUpEvent(SDL_KeyboardEvent keyEvent);
    void ResetKeyboardStates(); // CALL THIS AT THE END OF AN UPDATE STEP

/** Gamepad **/
    GamepadState& GetGamepad(i32 playerNumber);

    void ProcessSDLControllerButtonDownEvent(SDL_ControllerButtonEvent gamepadButtonEvent);
    void ProcessSDLControllerButtonUpEvent(SDL_ControllerButtonEvent gamepadButtonEvent);
    void ProcessSDLControllerAxisEvent(SDL_ControllerAxisEvent gamepadAxisEvent);
    void ResetControllerStates(); // CALL THIS AT THE END OF AN UPDATE STEP
    void SDLControllerConnected(i32 deviceIndex);
    void SDLControllerRemoved(i32 joystickInstanceID);
    void SetInvertYAxis(bool invert);
}

