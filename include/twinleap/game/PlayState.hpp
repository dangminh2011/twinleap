#pragma once
#include "twinleap/audio/Sfx.hpp"
#include "twinleap/game/Particles.hpp"
#include "twinleap/game/Physics.hpp"
#include "twinleap/render/Renderer.hpp"
#include <array>
namespace twinleap
{
class PlayState
{
    Particles particles;
    Vec2 checkpoint{};
    bool hasCheckpoint{};
    float shake{};
    int theme{};
    bool splitView{};
    void renderWorld(Renderer &r, bool debug);

  public:
    Level level;
    std::array<Player, 2> players{};
    Vec2 camera{};
    int score{}, deaths{};
    float time{};
    bool complete{};
    bool load(const std::string &path, std::string &error);
    void update(const std::array<PlayerInput, 2> &input, float dt, Sfx &sfx);
    void render(Renderer &r, bool debug);
};
} // namespace twinleap
