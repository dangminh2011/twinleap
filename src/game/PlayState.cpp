#include "twinleap/game/PlayState.hpp"
#include "twinleap/game/Camera.hpp"
#include "twinleap/game/Entities.hpp"
#include "twinleap/render/Palette.hpp"
#include <cmath>
#include <cstdio>
#include <filesystem>
namespace twinleap
{
bool PlayState::load(const std::string &path, std::string &error)
{
    if (!level.load(path, error))
        return false;
    const auto filename = std::filesystem::path(path).stem().string();
    theme = filename.rfind("02_", 0) == 0 ? 1 : filename.rfind("03_", 0) == 0 ? 2 : filename.rfind("04_", 0) == 0 ? 3 : 0;
    players = {};
    for (size_t i = 0; i < 2; ++i)
        players[i].pos = players[i].previous = level.spawn[i];
    score = deaths = 0;
    time = shake = 0;
    complete = hasCheckpoint = splitView = false;
    particles = {};
    camera = {};
    return true;
}
void PlayState::update(const std::array<PlayerInput, 2> &input, float dt, Sfx &sfx)
{
    if (complete)
        return;
    time += dt;
    shake = std::max(0.f, shake - dt);
    int first = players[0].pos.y > players[1].pos.y ? 0 : 1;
    for (int n = 0; n < 2; ++n)
    {
        int i = n == 0 ? first : 1 - first;
        auto &p = players[size_t(i)];
        auto &other = players[size_t(1 - i)];
        if (p.status == Player::Status::Dead)
        {
            p.respawnTimer -= dt;
            if (p.respawnTimer <= 0)
            {
                p = Player{};
                p.pos = hasCheckpoint ? checkpoint : level.spawn[i];
                if (overlaps(p.bounds(), other.bounds()))
                    p.pos.x += 16;
                p.previous = p.pos;
            }
            continue;
        }
        bool grounded = p.onGround;
        Physics::step(p, input[size_t(i)], level, &other, dt);
        if (grounded && p.vel.y < 0)
            sfx.play(Sound::Jump);
        if (!grounded && p.onGround)
            particles.burst({p.pos.x + 6, p.pos.y + 14}, {130, 140, 180, 255}, 5);
        bool dead = p.pos.y > float(level.height * 16 + 64);
        for (int y = int(p.pos.y / 16); y <= int((p.pos.y + 14) / 16); ++y)
            for (int x = int(p.pos.x / 16); x <= int((p.pos.x + 12) / 16); ++x)
                if (level.tileAt(x, y) == '^' && overlaps(p.bounds(), {float(x * 16 + 2), float(y * 16 + 8), 12, 8}))
                    dead = true;
        if (dead)
        {
            particles.burst(p.pos, i == 0 ? Palette::player1 : Palette::player2, 25);
            p.status = Player::Status::Dead;
            p.respawnTimer = Physics::RESPAWN_DELAY;
            ++deaths;
            shake = 0.15f;
            sfx.play(Sound::Death);
            continue;
        }
        for (auto &o : level.objects)
            if (overlaps(p.bounds(), o.box))
            {
                if (o.kind == 'o' && !o.active)
                {
                    o.active = true;
                    score += 100;
                    particles.burst({o.box.x + 8, o.box.y + 8}, {255, 213, 74, 255}, 12);
                    sfx.play(Sound::Coin);
                }
                if (o.kind == 'C' && !o.active)
                {
                    for (auto &c : level.objects)
                        if (c.kind == 'C')
                            c.active = false;
                    o.active = true;
                    checkpoint = {o.box.x + 2, o.box.y + 2};
                    hasCheckpoint = true;
                }
            }
        if (overlaps(p.bounds(), level.exit))
        {
            p.status = Player::Status::AtExit;
            p.vel = {};
        }
        else if (p.status == Player::Status::AtExit)
            p.status = Player::Status::Alive;
    }
    if (updateDoors(level, players, dt))
        sfx.play(Sound::Door);
    if (players[0].status == Player::Status::AtExit && players[1].status == Player::Status::AtExit)
    {
        complete = true;
        sfx.play(Sound::Win);
    }
    Camera view;
    view.position = camera;
    view.update(players, level.width, level.height, dt);
    camera = view.position;
    const float separationX = std::abs(players[0].pos.x - players[1].pos.x);
    const float separationY = std::abs(players[0].pos.y - players[1].pos.y);
    const bool bothVisible = players[0].status != Player::Status::Dead &&
                             players[1].status != Player::Status::Dead;
    // Different entry/exit distances keep the layout stable near the boundary.
    splitView = bothVisible && (splitView ? separationX > 320 || separationY > 140
                                         : separationX > 400 || separationY > 180);
    if (!splitView)
    {
        // Keep living players visible immediately after a respawn or a layout change.
        for (const auto &p : players)
            if (p.status != Player::Status::Dead)
            {
                camera.x = std::clamp(camera.x, p.pos.x + 28 - 480, p.pos.x - 16);
                camera.y = std::clamp(camera.y, p.pos.y + 30 - 270, p.pos.y - 44);
            }
        camera.x = level.width * 16 < 480 ? float(level.width * 16 - 480) / 2
                                         : std::clamp(camera.x, 0.f, float(level.width * 16 - 480));
        camera.y = level.height * 16 < 270 ? float(level.height * 16 - 270) / 2
                                          : std::clamp(camera.y, 0.f, float(level.height * 16 - 270));
    }
    for (auto &p : players)
        if (!splitView && p.status == Player::Status::Alive)
        {
            // Block movement away from the view, but let the camera catch up to a respawn.
            const float left = std::max(0.f, std::min(p.previous.x, camera.x + 16));
            const float right = std::min(float(level.width * 16 - 12),
                                         std::max(p.previous.x, camera.x + 452));
            p.pos.x = std::clamp(p.pos.x, left, right);
        }
    particles.update(dt);
}
void PlayState::renderWorld(Renderer &r, bool debug)
{
    if (shake > 0)
    {
        r.camera.x += std::sin(time * 130) * 2;
        r.camera.y += std::cos(time * 157) * 2;
    }
    r.background(theme);
    for (int y = 0; y < level.height; ++y)
        for (int x = 0; x < level.width; ++x)
        {
            float px = float(x * 16), py = float(y * 16);
            char c = level.tileAt(x, y);
            if (c == '#')
            {
                r.terrain(x, y, level.tileAt(x, y - 1) != '#', level.tileAt(x - 1, y) != '#',
                          level.tileAt(x + 1, y) != '#', theme);
            }
            if (c == '=')
            {
                r.sprite(Sprite::Terrain, {272, 16, 16, 4}, {px, py, 16, 4});
                r.rect({px + 2, py + 4, 12, 1}, {72, 60, 58, 255});
            }
            if (c == '^')
                r.sprite(Sprite::Spikes, {0, 0, 16, 16}, {px, py, 16, 16});
        }
    for (const auto &o : level.objects)
    {
        auto b = o.box;
        if (o.kind == 'o' && !o.active)
            r.rect({b.x + 5, b.y + 4, 6, 8}, {255, 213, 74, 255});
        if (o.kind == 'C')
        {
            r.rect({b.x + 3, b.y, 2, 16}, {180, 180, 190, 255});
            r.rect({b.x + 5, b.y, 9, 6}, o.active ? SDL_Color{80, 230, 140, 255} : SDL_Color{120, 120, 140, 255});
        }
        if (o.kind == 'a' || o.kind == 'b')
        {
            b.y += o.active ? 14.f : 12.f;
            b.h = o.active ? 2.f : 4.f;
            r.rect(b, o.kind == 'a' ? SDL_Color{255, 159, 67, 255} : SDL_Color{46, 211, 183, 255});
        }
        if (o.kind == 'A' || o.kind == 'B')
        {
            if (o.active)
                b.w = 2;
            r.rect(b, o.kind == 'A' ? SDL_Color{255, 159, 67, 255} : SDL_Color{46, 211, 183, 255});
        }
    }
    r.rect(level.exit, {176, 124, 255, 255});
    r.rect({level.exit.x + 3, level.exit.y + 3, 10, 13}, {65, 40, 110, 255});
    for (size_t i = 0; i < 2; ++i)
    {
        const auto &p = players[i];
        if (p.status == Player::Status::Dead)
            continue;
        auto b = p.bounds();
        r.character(int(i), b, p.vel.x, p.vel.y, p.onGround, p.facing, time, p.status == Player::Status::AtExit);
        if (debug)
        {
            r.rect({b.x, b.y, 12, 1}, {0, 255, 0, 255});
            r.rect({b.x, b.y, 1, 14}, {0, 255, 0, 255});
            r.line({b.x + 12, b.y}, {b.x + 12, b.y + 14}, {0, 255, 0, 255});
            r.line({b.x, b.y + 14}, {b.x + 12, b.y + 14}, {0, 255, 0, 255});
            r.line({b.x + 6, b.y + 7},
                   {b.x + 6 + p.vel.x * 0.1f, b.y + 7 + p.vel.y * 0.1f}, {255, 255, 0, 255});
        }
    }
    particles.render(r);
}
void PlayState::render(Renderer &r, bool debug)
{
    if (splitView)
    {
        auto bound = [](float value, float size, float screen) {
            return size < screen ? (size - screen) / 2 : std::clamp(value, 0.f, size - screen);
        };
        for (size_t i = 0; i < players.size(); ++i)
        {
            const SDL_Rect area{int(i) * 240, 30, 240, 240};
            r.viewport(&area);
            r.camera = {bound(players[i].pos.x + 6 - 120, float(level.width * 16), 240),
                        bound(players[i].pos.y + 7 - 120, float(level.height * 16), 240)};
            renderWorld(r, debug);
        }
        r.viewport(nullptr);
        r.rect({239, 30, 2, 240}, {20, 23, 43, 255}, false);
        r.rect({2, 256, 102, 14}, {20, 23, 43, 255}, false);
        r.rect({376, 256, 102, 14}, {20, 23, 43, 255}, false);
        r.text(6, 259, "P1 VIEW", {255, 138, 180, 255});
        r.text(416, 259, "P2 VIEW", {80, 210, 255, 255});
    }
    else
    {
        r.camera = camera;
        renderWorld(r, debug);
    }
    r.camera = camera;
    r.rect({0, 0, 480, 30}, {20, 23, 43, 255}, false);
    r.text(4, 4,
           players[0].status == Player::Status::Dead     ? "P1 DEAD"
           : players[0].status == Player::Status::AtExit ? "P1 EXIT"
                                                         : "P1",
           {255, 138, 180, 255});
    r.text(420, 4,
           players[1].status == Player::Status::Dead     ? "P2 DEAD"
           : players[1].status == Player::Status::AtExit ? "P2 EXIT"
                                                         : "P2",
           {80, 210, 255, 255});
    r.centered(3, level.name);
    char text[100];
    std::snprintf(text, sizeof(text), "Score %d  Deaths %d  %02d:%04.1f", score, deaths, int(time) / 60,
                  std::fmod(time, 60.f));
    r.centered(17, text);
    if (debug)
    {
        int solids = 0;
        for (const auto &p : players)
        {
            int tx = int(p.pos.x / 16), ty = int(p.pos.y / 16);
            for (int y = ty - 2; y <= ty + 2; ++y)
                for (int x = tx - 2; x <= tx + 2; ++x)
                    solids += level.isSolidAt(x, y) ? 1 : 0;
        }
        std::snprintf(text, sizeof(text), "Nearby solids %d", solids);
        r.text(5, 238, text);
    }
}
} // namespace twinleap
