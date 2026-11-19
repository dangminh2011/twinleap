#pragma once
#include "twinleap/game/Player.hpp"
#include <SDL3/SDL.h>
#include <array>
namespace twinleap
{
class Input
{
    std::array<SDL_Gamepad *, 2> pads{};
    std::array<bool, 2> held{};

  public:
    std::array<PlayerInput, 2> players{};
    ~Input();
    void event(const SDL_Event &e);
    void poll();
    void consume();
    bool confirm{}, pause{}, up{}, down{};
};
} // namespace twinleap
