#include "twinleap/game/MenuState.hpp"
#include "twinleap/game/PlayState.hpp"
#include <SDL3/SDL.h>
#include <array>
#include <cstdio>

int main(int argc, char **argv)
{
    if (argc != 2 || !SDL_SetHint(SDL_HINT_AUDIO_DRIVER, "dummy") || !SDL_Init(0))
        return 1;
    SDL_Surface *surface = SDL_CreateSurface(480, 270, SDL_PIXELFORMAT_RGBA32);
    SDL_Renderer *software = surface ? SDL_CreateSoftwareRenderer(surface) : nullptr;
    bool passed = software != nullptr;
    if (software)
    {
        twinleap::Renderer renderer(software);
        passed = renderer.assets.load(software);
        constexpr std::array<const char *, 4> files{
            "01_first_steps.txt", "02_buttons.txt", "03_tower.txt", "04_gauntlet.txt"};
        for (std::size_t i = 0; passed && i < files.size(); ++i)
        {
            twinleap::PlayState game;
            std::string error;
            passed = game.load(std::string(argv[1]) + "/" + files[i], error);
            if (!passed)
            {
                std::fprintf(stderr, "%s\n", error.c_str());
                break;
            }
            game.time = 0.25f;
            game.render(renderer, true);
            for (int player = 0; player < 2; ++player)
                for (int state = 0; state < 4; ++state)
                    renderer.character(player, {float(40 + state * 55), float(60 + player * 30), 12, 14},
                                       state == 1 ? 100.f : 0.f, state == 2 ? -100.f : 100.f,
                                       state < 2, player == 0 ? 1 : -1, 0.25f, state == 0);
            passed = !renderer.failed && SDL_RenderPresent(software);
            if (passed)
            {
                const auto preview = "render-preview-" + std::to_string(i + 1) + ".bmp";
                passed = SDL_SaveBMP(surface, preview.c_str());
            }
        }
        if (passed)
        {
            twinleap::Sfx sfx;
            twinleap::PlayState game;
            std::string error;
            passed = game.load(std::string(argv[1]) + "/05_last_ascent.txt", error);
            if (passed)
            {
                game.players[0].pos = {49 * 16.f + 2, 226};
                game.players[1].pos = {144 * 16.f + 2, 226};
                game.update({}, 0, sfx);
                game.render(renderer, false);
                SDL_Rect viewport{};
                Uint8 red{}, green{}, blue{}, alpha{};
                passed = !renderer.failed && SDL_RenderPresent(software) &&
                         SDL_GetRenderViewport(software, &viewport) &&
                         viewport.x == 0 && viewport.y == 0 && viewport.w == 480 && viewport.h == 270 &&
                         SDL_ReadSurfacePixel(surface, 239, 100, &red, &green, &blue, &alpha) &&
                         red == 20 && green == 23 && blue == 43 &&
                         SDL_SaveBMP(surface, "render-preview-split.bmp");
                // Stay split near the threshold, then merge when both players are close.
                game.players[0].pos.y = game.players[1].pos.y = 160;
                game.players[1].pos.x = game.players[0].pos.x + 350;
                game.update({}, 0, sfx);
                game.render(renderer, false);
                passed = passed && SDL_RenderPresent(software) &&
                         SDL_ReadSurfacePixel(surface, 239, 100, &red, &green, &blue, &alpha) &&
                         red == 20 && green == 23 && blue == 43;
                game.players[1].pos.x = game.players[0].pos.x + 100;
                game.update({}, 0, sfx);
                game.render(renderer, false);
                passed = passed && !renderer.failed && SDL_RenderPresent(software) &&
                         SDL_ReadSurfacePixel(surface, 239, 100, &red, &green, &blue, &alpha) &&
                         !(red == 20 && green == 23 && blue == 43);
            }
        }
        if (passed)
        {
            renderer.camera = {};
            renderer.background();
            twinleap::PlayState game;
            twinleap::MenuState::render(renderer, twinleap::Screen::Menu, 0, {}, game);
            passed = !renderer.failed && SDL_RenderPresent(software);
        }
    }
    if (!passed)
        std::fprintf(stderr, "Render assets: %s\n", SDL_GetError());
    if (software)
        SDL_DestroyRenderer(software);
    if (surface)
        SDL_DestroySurface(surface);
    SDL_Quit();
    return passed ? 0 : 1;
}
