#include "twinleap/audio/Sfx.hpp"
#include <algorithm>
#include <cstring>
#include <filesystem>
#include <limits>
namespace twinleap
{
namespace
{
bool loadSample(const char *filename, std::vector<float> &sample, float gain = 0.35f)
{
    std::vector<std::filesystem::path> roots{"assets/audio"};
    if (const char *base = SDL_GetBasePath())
    {
        roots.emplace_back(std::filesystem::path(base) / "assets/audio");
        roots.emplace_back(std::filesystem::path(base) / "../assets/audio");
    }
    std::error_code error;
    for (const auto &root : roots)
    {
        const auto path = root / filename;
        if (!std::filesystem::is_regular_file(path, error))
            continue;
        SDL_AudioSpec source{};
        Uint8 *raw = nullptr;
        Uint32 rawBytes = 0;
        if (!SDL_LoadWAV(path.string().c_str(), &source, &raw, &rawBytes))
        {
            SDL_Log("WAV %s: %s", filename, SDL_GetError());
            continue;
        }
        if (rawBytes == 0 || rawBytes > Uint32(std::numeric_limits<int>::max()))
        {
            SDL_Log("WAV %s: empty or too large", filename);
            SDL_free(raw);
            continue;
        }
        const SDL_AudioSpec target{SDL_AUDIO_F32, 1, 44100};
        Uint8 *converted = nullptr;
        int convertedBytes = 0;
        const bool ok = SDL_ConvertAudioSamples(&source, raw, int(rawBytes), &target, &converted, &convertedBytes);
        SDL_free(raw);
        if (!ok)
            SDL_Log("Convert WAV %s: %s", filename, SDL_GetError());
        else if (convertedBytes <= 0 || convertedBytes % int(sizeof(float)) != 0)
            SDL_Log("WAV %s: invalid converted size", filename);
        else
        {
            sample.resize(size_t(convertedBytes) / sizeof(float));
            std::memcpy(sample.data(), converted, size_t(convertedBytes));
            // Leave headroom for simultaneous effects from both players.
            for (float &value : sample)
                value *= gain;
        }
        SDL_free(converted);
        if (!sample.empty())
            return true;
    }
    SDL_Log("Sound %s unavailable; this effect will be silent", filename);
    return false;
}
} // namespace
Sfx::Sfx()
{
    if (!SDL_InitSubSystem(SDL_INIT_AUDIO))
    {
        SDL_Log("Audio disabled: %s", SDL_GetError());
        return;
    }
    constexpr std::array<const char *, 5> filenames{
        "sfx/jump.wav", "sfx/coin.wav", "sfx/death.wav", "sfx/door.wav", "sfx/win.wav"};
    for (size_t i = 0; i < filenames.size(); ++i)
        loadSample(filenames[i], samples[i]);
    loadSample("music/background.wav", music, 0.12f);
    SDL_AudioSpec spec{SDL_AUDIO_F32, 1, 44100};
    stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, nullptr, nullptr);
    if (!stream)
    {
        SDL_Log("Audio disabled: %s", SDL_GetError());
        return;
    }
    if (!SDL_ResumeAudioStreamDevice(stream))
    {
        SDL_Log("Audio disabled: %s", SDL_GetError());
        SDL_DestroyAudioStream(stream);
        stream = nullptr;
    }
}
Sfx::~Sfx()
{
    if (stream)
        SDL_DestroyAudioStream(stream);
}
void Sfx::play(Sound s)
{
    if (muted || paused || !stream || !hasSound(s))
        return;
    for (auto &v : voices)
        if (v.sound < 0)
        {
            v = {int(s), 0};
            return;
        }
    voices[0] = {int(s), 0};
}
bool Sfx::hasSound(Sound sound) const
{
    const auto index = size_t(sound);
    return index < samples.size() && !samples[index].empty();
}
bool Sfx::hasMusic() const
{
    return !music.empty();
}
void Sfx::setPaused(bool value)
{
    if (paused == value)
        return;
    paused = value;
    if (!stream)
        return;
    const bool ok = paused ? SDL_PauseAudioStreamDevice(stream) : SDL_ResumeAudioStreamDevice(stream);
    if (!ok)
        SDL_Log("Audio pause/resume: %s", SDL_GetError());
}
void Sfx::silence()
{
    for (auto &v : voices)
        v.sound = -1;
    if (stream && !SDL_ClearAudioStream(stream))
        SDL_Log("Audio clear: %s", SDL_GetError());
}
void Sfx::toggle()
{
    muted = !muted;
    silence();
}
void Sfx::update()
{
    if (!stream || muted || paused)
        return;
    int queued = SDL_GetAudioStreamQueued(stream);
    if (queued < 0)
    {
        SDL_Log("Audio queue: %s", SDL_GetError());
        return;
    }
    while (queued < 4096)
    {
        std::array<float, 512> buffer{};
        if (!music.empty())
            for (auto &x : buffer)
            {
                x = music[musicPosition++];
                if (musicPosition == music.size())
                    musicPosition = 0;
            }
        for (auto &v : voices)
            if (v.sound >= 0)
            {
                const auto &sample = samples[size_t(v.sound)];
                for (auto &x : buffer)
                {
                    if (v.position >= sample.size())
                    {
                        v.sound = -1;
                        break;
                    }
                    x += sample[v.position++];
                }
            }
        for (auto &x : buffer)
            x = std::clamp(x, -1.f, 1.f);
        if (!SDL_PutAudioStreamData(stream, buffer.data(), int(sizeof(buffer))))
        {
            SDL_Log("Audio write: %s", SDL_GetError());
            break;
        }
        queued += int(sizeof(buffer));
    }
}
} // namespace twinleap
