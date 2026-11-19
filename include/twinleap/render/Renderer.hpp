#pragma once
#include "twinleap/core/Math.hpp"
#include "twinleap/render/Assets.hpp"
#include <SDL3/SDL.h>
#include <string>
namespace twinleap
{
class Renderer
{
    SDL_Renderer *r;

  public:
    Assets assets;
    Vec2 camera{};
    bool failed{};
    explicit Renderer(SDL_Renderer *renderer) : r(renderer)
    {
    }
    void rect(Rect b, SDL_Color c, bool world = true);
    void line(Vec2 from,Vec2 to,SDL_Color color);
    void text(float x, float y, const char *text, SDL_Color c = {235, 237, 250, 255});
    void text(float x, float y, const std::string &s, SDL_Color c = {235, 237, 250, 255})
    {
        text(x, y, s.c_str(), c);
    }
    void centered(float y, const char *text);
    void centered(float y, const std::string &s)
    {
        centered(y, s.c_str());
    }
    void sprite(Sprite image, Rect source, Rect destination, bool world = true, bool flip = false, Uint8 alpha = 255);
    void character(int player, Rect hitbox, float velocityX, float velocityY, bool grounded, int facing, float time, bool atExit = false);
    void terrain(int x, int y, bool top, bool left, bool right, int theme);
    void background(int theme = 0);
    void check(bool success);
    void viewport(const SDL_Rect *area);
};
} // namespace twinleap
