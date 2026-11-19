#pragma once
#include <SDL3/SDL.h>
#include <array>
#include <vector>
namespace twinleap
{
enum class Sound
{
    Jump,
    Coin,
    Death,
    Door,
    Win
};
class Sfx
{
    SDL_AudioStream *stream{};
    bool muted{};
    bool paused{};
    std::array<std::vector<float>, 5> samples;
    std::vector<float> music;
    size_t musicPosition{};
    struct Voice
    {
        int sound{-1};
        size_t position{};
    };
    std::array<Voice, 8> voices{};

  public:
    Sfx();
    ~Sfx();
    bool hasSound(Sound sound) const;
    bool hasMusic() const;
    void setPaused(bool value);
    void play(Sound sound);
    void update();
    void silence();
    void toggle();
};
} // namespace twinleap
