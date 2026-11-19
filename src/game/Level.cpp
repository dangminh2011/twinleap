#include "twinleap/game/Level.hpp"
#include <array>
#include <fstream>
#include <sstream>
namespace twinleap
{
char Level::tileAt(int x, int y) const
{
    if (y >= height)
        return '.';
    if (x < 0 || y < 0 || x >= width)
        return '#';
    return tiles[static_cast<size_t>(y)][static_cast<size_t>(x)];
}
bool Level::isSolidAt(int x, int y) const
{
    if (tileAt(x, y) == '#')
        return true;
    Rect cell{float(x * 16), float(y * 16), 16, 16};
    for (const auto &o : objects)
        if ((o.kind == 'A' || o.kind == 'B') && !o.active && overlaps(cell, o.box))
            return true;
    return false;
}
bool Level::load(const std::string &path, std::string &error)
{
    *this = Level{};
    std::ifstream file(path);
    int lineNo = 0;
    bool map = false;
    std::array<int, 256> counts{};
    auto fail = [&](const std::string &reason) {
        error = path + ":" + std::to_string(lineNo) + ": " + reason;
        return false;
    };
    if (!file)
        return fail("cannot open level");
    std::string line;
    while (std::getline(file, line))
    {
        ++lineNo;
        if (lineNo == 1 && line.compare(0, 3, "\xEF\xBB\xBF") == 0)
            line.erase(0, 3);
        if (!line.empty() && line.back() == '\r')
            line.pop_back();
        if (!map)
        {
            if (line == "--")
            {
                map = true;
                continue;
            }
            if (line.empty() || line[0] == '#')
                continue;
            auto colon = line.find(':');
            if (colon == std::string::npos)
                return fail("expected key: value");
            auto value = line.substr(colon + 1);
            if (!value.empty() && value[0] == ' ')
                value.erase(0, 1);
            if (line.substr(0, colon) == "name")
                name = value;
            if (line.substr(0, colon) == "par")
            {
                std::istringstream in(value);
                if (!(in >> parTime) || parTime < 0)
                    return fail("invalid par");
            }
            continue;
        }
        if (line.empty())
            continue;
        for (size_t x = 0; x < line.size(); ++x)
        {
            char c = line[x];
            if (std::string(".#=^oCEabABPQ").find(c) == std::string::npos)
                return fail("invalid map character");
            ++counts[static_cast<unsigned char>(c)];
            Rect r{float(x * 16), float(tiles.size() * 16), 16, 16};
            if (c == 'P' || c == 'Q')
                spawn[c == 'P' ? 0 : 1] = {r.x + 2, r.y + 2};
            if (c == 'E')
                exit = r;
            if (std::string("oCabAB").find(c) != std::string::npos)
                objects.push_back({c, r, false, 0});
        }
        width = std::max(width, int(line.size()));
        tiles.push_back(line);
    }
    height = int(tiles.size());
    if (!map || !width || !height)
        return fail("empty map or missing --");
    for (char c : std::string("PQ"))
        if (counts[static_cast<unsigned char>(c)] != 1)
            return fail(std::string("expected exactly one ") + c);
    if (counts['E'] != 1)
        return fail("expected exactly one E");
    for (int i = 0; i < 2; ++i)
        if (bool(counts['a' + i]) != bool(counts['A' + i]))
            return fail("unpaired button/door");
    for (auto &row : tiles)
        row.resize(static_cast<size_t>(width), '.');
    if (name.empty())
        name = path;
    return true;
}
} // namespace twinleap
