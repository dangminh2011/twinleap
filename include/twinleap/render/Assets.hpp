#pragma once
#include <array>
#include <cstddef>

struct SDL_Renderer;
struct SDL_Texture;

namespace twinleap
{
enum class Sprite : std::size_t
{
    RaccoonIdle, RaccoonRun, RaccoonJump, RaccoonFall,
    BirdIdle, BirdRun, BirdJump, BirdFall,
    Terrain, Spikes, BackgroundBlue, BackgroundGreen, BackgroundPurple, BackgroundBrown,
    Count
};

class Assets
{
    std::array<SDL_Texture *, static_cast<std::size_t>(Sprite::Count)> textures{};

  public:
    Assets() = default;
    ~Assets();
    Assets(const Assets &) = delete;
    Assets &operator=(const Assets &) = delete;
    bool load(SDL_Renderer *renderer);
    SDL_Texture *get(Sprite sprite) const;
};
} // namespace twinleap
