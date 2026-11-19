#include "twinleap/game/MenuState.hpp"
#include <cstdio>
#include <cstring>
#include <initializer_list>
namespace twinleap
{
void MenuState::render(Renderer &r, Screen screen, int choice, const std::vector<std::string> &files,
                       const PlayState &game)
{
    auto menu = [&](std::initializer_list<const char *> labels, float y) {
        int i = 0;
        for (const char *label : labels)
        {
            r.centered(y + float(i * 18), label);
            if (i == choice)
                r.text(220 - float(std::strlen(label) * 4), y + float(i * 18), ">");
            ++i;
        }
    };
    if (screen == Screen::Menu)
    {
        r.rect({120, 94, 240, 90}, {35, 39, 62, 255}, false);
        r.character(0, {143, 204, 12, 14}, 0, 0, true, 1, float(SDL_GetTicksNS() / 1000000) / 1000.f);
        r.character(1, {325, 204, 12, 14}, 0, 0, true, -1, float(SDL_GetTicksNS() / 1000000) / 1000.f);
        menu({"Start", "Select Level", "Controls", "Quit"}, 110);
        r.text(110, 220, "Raccoon");
        r.text(290, 220, "Blue Bird");
        r.centered(235, "Enter / A: select    M: sound");
    }
    if (screen == Screen::Select)
    {
        for (size_t i = 0; i < files.size(); ++i)
            r.centered(100 + float(i * 18), files[i]);
    }
    if (screen == Screen::Select)
        r.text(15, 100 + float(choice * 18), ">");
    if (screen == Screen::Controls)
    {
        r.centered(105, "P1: A / D + W or Space");
        r.centered(125, "P2: Arrows + Up or Right Ctrl");
        r.centered(145, "Gamepad: Stick / D-pad + A");
        r.centered(165, "Esc Pause  R Restart  M Sound");
        r.centered(185, "F11 Fullscreen  F3 Debug");
        r.centered(220, "Stand on each other. Exit together.");
    }
    if (screen == Screen::Pause)
    {
        r.rect({100, 70, 280, 140}, {20, 23, 43, 255}, false);
        r.centered(85, "PAUSED");
        menu({"Resume", "Restart", "Menu"}, 115);
    }
    if (screen == Screen::Complete)
    {
        r.rect({65, 80, 350, 100}, {20, 23, 43, 255}, false);
        r.centered(95, "LEVEL COMPLETE!");
        char summary[100];
        std::snprintf(summary, sizeof(summary), "Score %d  Deaths %d  Time %.1fs", game.score, game.deaths, game.time);
        r.centered(120, summary);
        r.centered(145, "Enter / A: next level");
    }
    if (screen == Screen::Congrats)
    {
        r.centered(120, "CONGRATULATIONS!");
        r.centered(145, "You made it together.");
        r.centered(185, "Enter / A: main menu");
    }
    if (screen == Screen::Error)
    {
        r.centered(110, "Unable to load levels");
        r.centered(140, "Enter: quit");
    }
}
} // namespace twinleap
