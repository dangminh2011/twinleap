#include "twinleap/game/Level.hpp"
#include <filesystem>
#include <fstream>
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
    CHECK(argc == 2);
    int count = 0;
    std::string error;
    twinleap::Level level;
    for (auto &f : std::filesystem::directory_iterator(argv[1]))
        if (f.path().extension() == ".txt")
        {
            CHECK(level.load(f.path().string(), error));
            CHECK(level.width > 0 && level.height > 0);
            ++count;
        }
    CHECK(count >= 5);
    auto fixture = std::filesystem::current_path() / "parser_fixture.txt";
    auto parse = [&](const std::string &content) {
        std::ofstream out(fixture);
        out << content;
        out.close();
        return level.load(fixture.string(), error);
    };
    CHECK(parse("name: Test\n--\nP.Q.E\n##\n"));
    CHECK(level.width == 5 && level.tiles[1] == "##...");
    CHECK(!parse("--\nQ.E\n"));
    CHECK(error.find("P") != std::string::npos);
    CHECK(!parse("--\nP.E\n"));
    CHECK(error.find("Q") != std::string::npos);
    CHECK(!parse("--\nP.Q\n"));
    CHECK(error.find("E") != std::string::npos);
    CHECK(!parse("--\nPPQE\n"));
    CHECK(!parse("--\nPQEA\n"));
    CHECK(!parse("--\nPQEa\n"));
    CHECK(!parse("--\nPQE?\n"));
    CHECK(error.find(":2:") != std::string::npos);
    CHECK(!parse("par: bad\n--\nPQE\n"));
    CHECK(!parse("name: missing separator\n"));
    std::filesystem::remove(fixture);
    return 0;
}
