#include "twinleap/game/PlayState.hpp"
#include <iostream>
#define CHECK(x)                                                                                                       \
    do                                                                                                                 \
    {                                                                                                                  \
        if (!(x))                                                                                                      \
        {                                                                                                              \
            std::cerr << "line " << __LINE__ << ": " << #x;                                                            \
            return 1;                                                                                                  \
        }                                                                                                              \
    } while (false)
int main(int argc, char **argv)
{
    using namespace twinleap;
    CHECK(argc == 2);
    CHECK(SDL_SetHint(SDL_HINT_AUDIO_DRIVER, "dummy"));
    CHECK(SDL_Init(0));
    {
        Sfx sfx;
        CHECK(sfx.hasMusic());
        for (int i = 0; i < 5; ++i)
            CHECK(sfx.hasSound(Sound(i)));
        PlayState game;
        std::string error;
        CHECK(game.load(std::string(argv[1]) + "/01_first_steps.txt", error));
        std::array<PlayerInput, 2> inputs{};
        game.players[0].pos = {game.level.exit.x, game.level.exit.y};
        game.players[1].pos = {40, 80};
        game.update(inputs, 1.f / 120, sfx);
        CHECK(!game.complete);
        game.players[1].pos = {game.level.exit.x, game.level.exit.y};
        game.update(inputs, 1.f / 120, sfx);
        CHECK(game.complete);
        CHECK(game.load(std::string(argv[1]) + "/01_first_steps.txt", error));
        game.players[0].pos.y = float(game.level.height * 16 + 65);
        game.update(inputs, 1.f / 120, sfx);
        CHECK(game.deaths == 1 && game.players[0].status == Player::Status::Dead);
        for (int n = 0; n < 125; ++n)
            game.update(inputs, 1.f / 120, sfx);
        CHECK(game.players[0].status == Player::Status::Alive);
        CHECK(game.score == 0);
        CHECK(game.load(std::string(argv[1]) + "/03_tower.txt", error));
        for (const auto &o : game.level.objects)
            if (o.kind == 'C')
                game.players[0].pos = {o.box.x, o.box.y};
        game.update(inputs, 1.f / 120, sfx);
        game.players[1].pos.y = float(game.level.height * 16 + 65);
        game.update(inputs, 1.f / 120, sfx);
        for (int n = 0; n < 122; ++n)
            game.update(inputs, 1.f / 120, sfx);
        CHECK(game.players[1].pos.x > 180);
        // A camera left ahead of the respawn must not pull the player into a pit.
        CHECK(game.load(std::string(argv[1]) + "/05_last_ascent.txt", error));
        game.players[1].pos = {30 * 16.f + 2, 14 * 16.f + 2};
        game.players[0].status = Player::Status::Dead;
        game.players[0].respawnTimer = 0;
        game.camera.x = 230;
        game.update(inputs, 1.f / 120, sfx);
        CHECK(game.players[0].pos.x == game.level.spawn[0].x);
        CHECK(game.players[0].pos.y == game.level.spawn[0].y);
        for (int n = 0; n < 180; ++n)
            game.update(inputs, 1.f / 120, sfx);
        CHECK(game.players[0].status == Player::Status::Alive);
        CHECK(game.deaths == 0);
        const auto mapObjects = game.level.objects;
        for (const auto &o : mapObjects)
            if (o.kind == 'C')
            {
                CHECK(game.load(std::string(argv[1]) + "/05_last_ascent.txt", error));
                game.players[0].status = Player::Status::Dead;
                game.players[0].respawnTimer = 10;
                game.players[1].pos = {o.box.x + 2, o.box.y + 2};
                game.camera.x = o.box.x - 200;
                game.update(inputs, 1.f / 120, sfx);
                game.players[0].respawnTimer = 0;
                game.camera.x = o.box.x + 180;
                game.update(inputs, 1.f / 120, sfx);
                CHECK(game.players[0].pos.x == o.box.x + 18);
                CHECK(game.players[0].pos.y == o.box.y + 2);
                for (int n = 0; n < 180; ++n)
                    game.update(inputs, 1.f / 120, sfx);
                CHECK(game.players[0].status == Player::Status::Alive);
                CHECK(game.deaths == 0);
            }
        for (int i = 0; i < 5; ++i)
            sfx.play(Sound(i));
        for (int i = 0; i < 10; ++i)
            sfx.update();
        sfx.toggle();
        sfx.play(Sound::Death);
        sfx.toggle();
        sfx.silence();
        sfx.setPaused(true);
        sfx.update();
        sfx.setPaused(false);
        sfx.update();
    }
    SDL_Quit();
    return 0;
}
