#include "twinleap/core/Input.hpp"
#include <cmath>
namespace twinleap
{
Input::~Input()
{
    for (auto *p : pads)
        if (p)
            SDL_CloseGamepad(p);
}
void Input::event(const SDL_Event &e)
{
    if ((e.type == SDL_EVENT_GAMEPAD_BUTTON_DOWN || e.type == SDL_EVENT_GAMEPAD_BUTTON_UP) &&
        e.gbutton.button == SDL_GAMEPAD_BUTTON_SOUTH)
        for (size_t i = 0; i < pads.size(); ++i)
            if (pads[i] && SDL_GetGamepadID(pads[i]) == e.gbutton.which)
            {
                if (e.type == SDL_EVENT_GAMEPAD_BUTTON_DOWN)
                    players[i].jumpPressed = true;
                else
                    players[i].jumpReleased = true;
            }
    if ((e.type == SDL_EVENT_KEY_DOWN && !e.key.repeat) || e.type == SDL_EVENT_KEY_UP)
    {
        auto s = e.key.scancode;
        int i = (s == SDL_SCANCODE_W || s == SDL_SCANCODE_SPACE)    ? 0
                : (s == SDL_SCANCODE_UP || s == SDL_SCANCODE_RCTRL) ? 1
                                                                    : -1;
        if (i >= 0)
        {
            if (e.type == SDL_EVENT_KEY_DOWN)
                players[size_t(i)].jumpPressed = true;
            else
                players[size_t(i)].jumpReleased = true;
        }
    }
    if (e.type == SDL_EVENT_GAMEPAD_ADDED)
    {
        for (auto &p : pads)
            if (!p)
            {
                p = SDL_OpenGamepad(e.gdevice.which);
                if (!p)
                    SDL_Log("Gamepad: %s", SDL_GetError());
                break;
            }
    }
    if (e.type == SDL_EVENT_GAMEPAD_REMOVED)
        for (auto &p : pads)
            if (p && SDL_GetGamepadID(p) == e.gdevice.which)
            {
                SDL_CloseGamepad(p);
                p = nullptr;
            }
    if (e.type == SDL_EVENT_GAMEPAD_BUTTON_DOWN)
    {
        confirm |= e.gbutton.button == SDL_GAMEPAD_BUTTON_SOUTH;
        pause |= e.gbutton.button == SDL_GAMEPAD_BUTTON_START;
        up |= e.gbutton.button == SDL_GAMEPAD_BUTTON_DPAD_UP;
        down |= e.gbutton.button == SDL_GAMEPAD_BUTTON_DPAD_DOWN;
    }
    if (e.type == SDL_EVENT_KEY_DOWN && !e.key.repeat)
    {
        auto s = e.key.scancode;
        confirm |= s == SDL_SCANCODE_RETURN;
        pause |= s == SDL_SCANCODE_ESCAPE;
        up |= s == SDL_SCANCODE_UP || s == SDL_SCANCODE_W;
        down |= s == SDL_SCANCODE_DOWN || s == SDL_SCANCODE_S;
    }
}
void Input::poll()
{
    const bool *k = SDL_GetKeyboardState(nullptr);
    for (size_t i = 0; i < 2; ++i)
    {
        float x = float(k[i ? SDL_SCANCODE_RIGHT : SDL_SCANCODE_D]) - float(k[i ? SDL_SCANCODE_LEFT : SDL_SCANCODE_A]);
        bool jump = i ? (k[SDL_SCANCODE_UP] || k[SDL_SCANCODE_RCTRL]) : (k[SDL_SCANCODE_W] || k[SDL_SCANCODE_SPACE]);
        if (pads[i])
        {
            float axis = float(SDL_GetGamepadAxis(pads[i], SDL_GAMEPAD_AXIS_LEFTX)) / 32767.f;
            if (std::abs(axis) > 0.25f)
                x += axis;
            x += float(SDL_GetGamepadButton(pads[i], SDL_GAMEPAD_BUTTON_DPAD_RIGHT)) -
                 float(SDL_GetGamepadButton(pads[i], SDL_GAMEPAD_BUTTON_DPAD_LEFT));
            jump |= SDL_GetGamepadButton(pads[i], SDL_GAMEPAD_BUTTON_SOUTH);
        }
        players[i].moveX = std::clamp(x, -1.f, 1.f);
        players[i].jumpHeld = jump;
        players[i].jumpPressed |= jump && !held[i];
        players[i].jumpReleased = !jump && (players[i].jumpReleased || held[i]);
        held[i] = jump;
    }
}
void Input::consume()
{
    for (auto &p : players)
    {
        p.jumpPressed = false;
        p.jumpReleased = false;
    }
    confirm = pause = up = down = false;
}
} // namespace twinleap
