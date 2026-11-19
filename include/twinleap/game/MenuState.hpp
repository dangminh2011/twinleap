#pragma once
#include "twinleap/game/GameState.hpp"
#include "twinleap/game/PlayState.hpp"
#include <vector>
namespace twinleap
{
class MenuState
{
  public:
    static void render(Renderer &r, Screen screen, int choice, const std::vector<std::string> &files,
                       const PlayState &game);
};
} // namespace twinleap
