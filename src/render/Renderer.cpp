#include "twinleap/render/Renderer.hpp"
#include <cmath>
#include <cstring>
namespace twinleap
{
void Renderer::viewport(const SDL_Rect *area)
{
    check(SDL_SetRenderViewport(r, area));
}
void Renderer::check(bool ok)
{
    if (!ok)
    {
        SDL_Log("Render: %s", SDL_GetError());
        failed = true;
    }
}
void Renderer::rect(Rect b, SDL_Color c, bool world)
{
    if (world)
    {
        b.x -= camera.x;
        b.y -= camera.y;
    }
    SDL_FRect f{b.x, b.y, b.w, b.h};
    check(SDL_SetRenderDrawColor(r, c.r, c.g, c.b, c.a));
    check(SDL_RenderFillRect(r, &f));
}
void Renderer::line(Vec2 from, Vec2 to, SDL_Color c)
{
    check(SDL_SetRenderDrawColor(r, c.r, c.g, c.b, c.a));
    check(SDL_RenderLine(r, from.x - camera.x, from.y - camera.y,
                        to.x - camera.x, to.y - camera.y));
}
void Renderer::text(float x, float y, const char *s, SDL_Color c)
{
    check(SDL_SetRenderDrawColor(r, c.r, c.g, c.b, c.a));
    check(SDL_RenderDebugText(r, x, y, s));
}
void Renderer::centered(float y, const char *s)
{
    text((480 - float(std::strlen(s) * 8)) / 2, y, s);
}
void Renderer::sprite(Sprite image, Rect source, Rect destination, bool world, bool flip, Uint8 alpha)
{
    auto *texture = assets.get(image);
    if (!texture)
    {
        failed = true;
        return;
    }
    if (world)
    {
        destination.x = std::round(destination.x - camera.x);
        destination.y = std::round(destination.y - camera.y);
    }
    SDL_FRect src{source.x, source.y, source.w, source.h};
    SDL_FRect dst{destination.x, destination.y, destination.w, destination.h};
    check(SDL_SetTextureAlphaMod(texture, alpha));
    check(SDL_RenderTextureRotated(r, texture, &src, &dst, 0, nullptr,
                                  flip ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE));
}

void Renderer::character(int player, Rect hitbox, float velocityX, float velocityY, bool grounded,
                         int facing, float time, bool atExit)
{
    int state = grounded ? (std::abs(velocityX) > 5 ? 1 : 0) : (velocityY < 0 ? 2 : 3);
    if (atExit)
        state = 0;
    const int frame = state < 2 ? int(time * (state == 1 ? 12.f : 6.f)) % (state == 1 ? 8 : 4) : 0;
    auto image = static_cast<Sprite>(player * 4 + state);
    // The 32px cell includes transparent padding. Feet stay aligned with the physics hitbox.
    sprite(image, {float(frame * 32), 0, 32, 32},
           {hitbox.x - 6, hitbox.y + hitbox.h - 21, 24, 24}, true, facing < 0, atExit ? 140 : 255);
    const SDL_Color color = player == 0 ? SDL_Color{255, 138, 180, 255} : SDL_Color{80, 210, 255, 255};
    text(std::round(hitbox.x - camera.x - 2), std::round(hitbox.y - camera.y - 12), player == 0 ? "1" : "2", color);
}

void Renderer::terrain(int x, int y, bool top, bool left, bool right, int theme)
{
    const float atlasX = theme == 3 ? 192.f : 96.f;
    const float atlasY = theme == 1 || theme == 3 ? 64.f : theme == 2 ? 128.f : 0.f;
    const float column = left ? 0.f : right ? 32.f : 16.f;
    sprite(Sprite::Terrain, {atlasX + column, atlasY + (top ? 0.f : 16.f), 16, 16},
           {float(x * 16), float(y * 16), 16, 16});
}

void Renderer::background(int theme)
{
    constexpr Sprite backgrounds[]{Sprite::BackgroundBlue, Sprite::BackgroundGreen,
                                   Sprite::BackgroundPurple, Sprite::BackgroundBrown};
    const float offsetX = std::fmod(camera.x * 0.2f, 64.f);
    const float offsetY = std::fmod(camera.y * 0.2f, 64.f);
    for (int y = -1; y < 6; ++y)
        for (int x = -1; x < 9; ++x)
            sprite(backgrounds[theme % 4], {0, 0, 64, 64},
                   {float(x * 64) - offsetX, float(y * 64) - offsetY, 64, 64}, false);
    // Distant silhouettes move more slowly than the playable terrain.
    for (int i = -1; i < 10; ++i)
    {
        const float x = float(i * 80) - std::fmod(camera.x * 0.45f, 80.f);
        const float height = 22.f + float((i * i + 3) % 5) * 9.f;
        rect({x, 270 - height, 78, height}, {80, 90, 115, 255}, false);
        rect({x, 270 - height, 78, 3}, {125, 145, 160, 255}, false);
    }
}
} // namespace twinleap
