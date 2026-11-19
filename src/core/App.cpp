#include "twinleap/core/App.hpp"
#include "twinleap/core/Input.hpp"
#include "twinleap/core/Time.hpp"
#include "twinleap/game/GameState.hpp"
#include "twinleap/game/MenuState.hpp"
#include "twinleap/game/PlayState.hpp"
#include <SDL3/SDL.h>
#include <cstdio>
#include <filesystem>
#include <string>
#include <vector>
namespace twinleap
{
namespace
{
void showStartupError(const char *context, SDL_Window *window = nullptr)
{
    const std::string detail = std::string(context) + ": " + SDL_GetError();
    SDL_Log("%s", detail.c_str());
    if (!SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Twinleap", detail.c_str(), window))
        SDL_Log("Error dialog: %s", SDL_GetError());
}
} // namespace
int App::run(int argc, char **argv)
{
    bool smoke = argc > 1 && std::string(argv[1]) == "--smoke";
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD))
    {
        showStartupError("Cannot initialize SDL");
        return 1;
    }
    SDL_Window *window = SDL_CreateWindow("Twinleap - Together we leap", 1280, 720, SDL_WINDOW_RESIZABLE);
    SDL_Renderer *renderer = window ? SDL_CreateRenderer(window, nullptr) : nullptr;
    if (!renderer)
    {
        showStartupError("Cannot create the game window", window);
        if (window)
            SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    int result = 0;
    {
        Renderer r(renderer);
        r.check(r.assets.load(renderer));
        r.check(SDL_SetRenderVSync(renderer, 1));
        r.check(SDL_SetRenderLogicalPresentation(renderer, 480, 270, SDL_LOGICAL_PRESENTATION_LETTERBOX));
        Input input;
        Sfx sfx;
        PlayState game;
        std::vector<std::string> files;
        std::error_code ec;
        const char *base = SDL_GetBasePath();
        std::vector<std::filesystem::path> roots = {"assets/maps"};
        if (base)
        {
            roots.emplace_back(std::filesystem::path(base) / "assets/maps");
            roots.emplace_back(std::filesystem::path(base) / "../assets/maps");
        }
        for (const auto &root : roots)
        {
            if (!std::filesystem::is_directory(root, ec))
                continue;
            for (std::filesystem::directory_iterator it(root, ec), end; !ec && it != end; it.increment(ec))
                if (it->path().extension() == ".txt")
                    files.push_back(it->path().string());
            if (!files.empty())
                break;
        }
        std::sort(files.begin(), files.end());
        std::vector<std::string> displayNames;
        for (const auto &file : files)
            displayNames.push_back(std::filesystem::path(file).stem().string());
        std::string error;
        Screen screen = files.empty() ? Screen::Error : Screen::Menu;
        Screen requested = screen;
        int choice = 0;
        size_t current = 0;
        bool running = true, fullscreen = false, debug = false;
        int frames = 0;
        Time clock;
        Uint64 previous = SDL_GetTicksNS();
        float fps = 0;
        auto load = [&]() {
            if (game.load(files[current], error))
                requested = Screen::Play;
            else
            {
                SDL_Log("%s", error.c_str());
                requested = Screen::Error;
            }
            input.consume();
        };
        if (files.empty())
        {
            error = "No assets/maps directory or .txt maps found";
            SDL_Log("%s", error.c_str());
        }
        if (smoke && !files.empty())
        {
            load();
            screen = requested;
        }
        while (running && !r.failed)
        {
            SDL_Event event;
            while (SDL_PollEvent(&event))
            {
                input.event(event);
                if (event.type == SDL_EVENT_QUIT)
                    running = false;
                if (event.type == SDL_EVENT_KEY_DOWN && !event.key.repeat)
                {
                    auto key = event.key.scancode;
                    if (key == SDL_SCANCODE_F11)
                    {
                        fullscreen = !fullscreen;
                        r.check(SDL_SetWindowFullscreen(window, fullscreen));
                    }
                    if (key == SDL_SCANCODE_F3)
                        debug = !debug;
                    if (key == SDL_SCANCODE_M)
                        sfx.toggle();
                    if (key == SDL_SCANCODE_R && (screen == Screen::Play || screen == Screen::Pause))
                        load();
                }
            }
            input.poll();
            Uint64 now = SDL_GetTicksNS();
            float elapsed = std::min(float(now - previous) / 1e9f, 0.25f);
            previous = now;
            if (elapsed > 0)
                fps += ((1 / elapsed) - fps) * 0.05f;
            if (SDL_GetWindowFlags(window) & SDL_WINDOW_MINIMIZED)
            {
                clock.reset();
                sfx.silence();
                sfx.setPaused(true);
                SDL_Delay(16);
                continue;
            }
            if (input.pause)
            {
                if (screen == Screen::Play)
                {
                    requested = Screen::Pause;
                    choice = 0;
                    sfx.silence();
                }
                else if (screen == Screen::Pause)
                    requested = Screen::Play;
                else if (screen != Screen::Menu)
                {
                    requested = Screen::Menu;
                    choice = 0;
                }
                input.pause = false;
            }
            if (screen != Screen::Play)
            {
                int count = screen == Screen::Menu     ? 4
                            : screen == Screen::Pause  ? 3
                            : screen == Screen::Select ? int(files.size())
                                                       : 1;
                if (input.up)
                    choice = (choice + count - 1) % count;
                if (input.down)
                    choice = (choice + 1) % count;
                if (input.confirm)
                {
                    if (screen == Screen::Menu)
                    {
                        int selected = choice;
                        if (selected == 0)
                        {
                            current = 0;
                            load();
                        }
                        else if (selected == 1)
                        {
                            requested = Screen::Select;
                            choice = 0;
                        }
                        else if (selected == 2)
                            requested = Screen::Controls;
                        else if (selected == 3)
                            running = false;
                    }
                    else if (screen == Screen::Select)
                    {
                        current = size_t(choice);
                        load();
                    }
                    else if (screen == Screen::Pause)
                    {
                        if (choice == 0)
                            requested = Screen::Play;
                        else if (choice == 1)
                            load();
                        else
                        {
                            requested = Screen::Menu;
                            choice = 0;
                        }
                    }
                    else if (screen == Screen::Complete)
                    {
                        if (current + 1 < files.size())
                        {
                            ++current;
                            load();
                        }
                        else
                            requested = Screen::Congrats;
                    }
                    else if (screen == Screen::Congrats || screen == Screen::Controls)
                    {
                        requested = Screen::Menu;
                        choice = 0;
                    }
                    else if (screen == Screen::Error)
                        running = false;
                }
                input.consume();
                clock.reset();
            }
            else
            {
                clock.add(elapsed);
                while (clock.ready())
                {
                    game.update(input.players, 1.f / 120, sfx);
                    input.consume();
                    clock.consume();
                    if (game.complete)
                    {
                        requested = Screen::Complete;
                        break;
                    }
                }
            }
            screen = requested;
            sfx.setPaused(screen == Screen::Pause);
            if (screen == Screen::Play || screen == Screen::Pause || screen == Screen::Complete)
                game.render(r, debug);
            else
            {
                r.camera = {};
                r.background();
                r.centered(45, "T W I N L E A P");
                r.centered(65, "Together we leap");
            }
            MenuState::render(r, screen, choice, displayNames, game);
            if (screen == Screen::Error)
            {
                for (size_t offset = 0, lineNumber = 0; offset < error.size() && lineNumber < 5;
                     offset += 56, ++lineNumber)
                {
                    char line[57]{};
                    std::copy_n(error.data() + offset, std::min(size_t(56), error.size() - offset), line);
                    r.text(12, 170 + float(lineNumber * 12), line);
                }
            }
            if (debug)
            {
                char stats[60];
                std::snprintf(stats, sizeof(stats), "FPS %d  F3 Debug", int(fps));
                r.text(5, 250, stats);
            }
            r.check(SDL_RenderPresent(renderer));
            if (screen != Screen::Pause)
                sfx.update();
            if (smoke && ++frames > 30)
                running = false;
        }
        if (r.failed && !smoke)
            showStartupError("Cannot render the game", window);
        if (r.failed || screen == Screen::Error)
            result = 1;
    }
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return result;
}
} // namespace twinleap
