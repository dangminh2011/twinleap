#include "twinleap/game/Entities.hpp"
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
int main()
{
    using namespace twinleap;
    Level l;
    l.objects = {{'a', {16, 16, 16, 16}}, {'A', {64, 16, 16, 16}}};
    std::array<Player, 2> p{};
    p[0].pos = {18, 18};
    p[0].onGround = true;
    p[1].pos = {100, 0};
    CHECK(updateDoors(l, p, 0.01f));
    CHECK(l.objects[1].active);
    p[0].pos = {0, 0};
    updateDoors(l, p, 0.1f);
    CHECK(l.objects[1].active);
    updateDoors(l, p, 0.11f);
    CHECK(!l.objects[1].active);
    p[0].pos = {18, 18};
    updateDoors(l, p, 0.01f);
    p[0].pos = {65, 18};
    updateDoors(l, p, 1);
    CHECK(l.objects[1].active);
    p[0].pos = {100, 0};
    updateDoors(l, p, 1);
    CHECK(!l.objects[1].active);
    return 0;
}
