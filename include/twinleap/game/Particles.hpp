#pragma once
#include "twinleap/render/Renderer.hpp"
#include <array>
namespace twinleap
{
class Particles
{
    struct Particle
    {
        Vec2 pos{}, vel{};
        float life{};
        SDL_Color color{};
    };
    std::array<Particle, 512> pool{};
    unsigned random{1729};

  public:
    void burst(Vec2 pos, SDL_Color color, int count);
    void update(float dt);
    void render(Renderer &r) const;
};
} // namespace twinleap
