#pragma once
#include "twinleap/core/Math.hpp"
#include <string>
#include <vector>
namespace twinleap
{
struct Object
{
    char kind{};
    Rect box{};
    bool active{};
    float timer{};
};
struct Level
{
    int width{}, height{};
    std::string name;
    float parTime{};
    std::vector<std::string> tiles;
    std::vector<Object> objects;
    Vec2 spawn[2]{};
    Rect exit{};
    char tileAt(int x, int y) const;
    bool isSolidAt(int x, int y) const;
    bool load(const std::string &path, std::string &error);
};
} // namespace twinleap
