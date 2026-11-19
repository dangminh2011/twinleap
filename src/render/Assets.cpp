#include "twinleap/render/Assets.hpp"
#include <SDL3/SDL.h>
#include <filesystem>
#include <vector>

namespace twinleap
{
Assets::~Assets()
{
    for (auto *texture : textures)
        if (texture)
            SDL_DestroyTexture(texture);
}

bool Assets::load(SDL_Renderer *renderer)
{
    constexpr std::array<const char *, 14> names{
        "characters/raccoon/idle", "characters/raccoon/run", "characters/raccoon/jump", "characters/raccoon/fall",
        "characters/bird/idle", "characters/bird/run", "characters/bird/jump", "characters/bird/fall",
        "terrain/terrain", "traps/spikes", "backgrounds/blue", "backgrounds/green", "backgrounds/purple", "backgrounds/brown"};
    std::vector<std::filesystem::path> roots{"assets"};
    if (const char *base = SDL_GetBasePath())
    {
        roots.emplace_back(std::filesystem::path(base) / "assets");
        roots.emplace_back(std::filesystem::path(base) / "../assets");
    }
    std::filesystem::path root;
    std::error_code error;
    for (const auto &candidate : roots)
        if (std::filesystem::is_regular_file(candidate / "characters/raccoon/idle.bmp", error))
        {
            root = candidate;
            break;
        }
    if (root.empty())
    {
        SDL_SetError("Missing assets/characters/raccoon/idle.bmp. Keep assets beside twinleap.exe.");
        SDL_Log("Assets: %s", SDL_GetError());
        return false;
    }
    for (std::size_t i = 0; i < names.size(); ++i)
    {
        const auto path = (root / (std::string(names[i]) + ".bmp")).string();
        SDL_Surface *surface = SDL_LoadBMP(path.c_str());
        if (!surface)
        {
            SDL_Log("Asset %s: %s", path.c_str(), SDL_GetError());
            return false;
        }
        const bool keyed = SDL_SetSurfaceColorKey(surface, true, SDL_MapSurfaceRGB(surface, 255, 0, 255));
        if (keyed)
            textures[i] = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_DestroySurface(surface);
        if (!keyed || !textures[i] ||
            !SDL_SetTextureScaleMode(textures[i], SDL_SCALEMODE_NEAREST) ||
            !SDL_SetTextureBlendMode(textures[i], SDL_BLENDMODE_BLEND))
        {
            SDL_Log("Asset %s: %s", path.c_str(), SDL_GetError());
            return false;
        }
    }
    return true;
}

SDL_Texture *Assets::get(Sprite sprite) const
{
    return textures[static_cast<std::size_t>(sprite)];
}
} // namespace twinleap
