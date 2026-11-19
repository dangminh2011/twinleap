#include "twinleap/game/Physics.hpp"
#include <iostream>
#define CHECK(x)                                                                                                       \
    do                                                                                                                 \
    {                                                                                                                  \
        if (!(x))                                                                                                      \
        {                                                                                                              \
            std::cerr << "line " << __LINE__ << ": " << #x << '\n';                                                    \
            return 1;                                                                                                  \
        }                                                                                                              \
    } while (false)
using namespace twinleap;
constexpr float dt = 1.f / 120;
static Level arena()
{
    Level l;
    l.width = 30;
    l.height = 20;
    l.tiles.assign(20, std::string(30, '.'));
    l.tiles[15] = std::string(30, '#');
    return l;
}
static void step(Player &p, const Level &l, int n, PlayerInput in = {})
{
    for (int i = 0; i < n; ++i)
        Physics::step(p, in, l, nullptr, dt);
}
static float jumpHeight(bool cut)
{
    auto l = arena();
    Player p;
    p.pos = {40, 226};
    p.onGround = true;
    Physics::step(p, {0, true, true, false}, l, nullptr, dt);
    float peak = p.pos.y;
    for (int i = 0; i < 120; ++i)
    {
        Physics::step(p, {0, false, !cut, cut && i == 5}, l, nullptr, dt);
        peak = std::min(peak, p.pos.y);
    }
    return 226 - peak;
}
int main()
{
    auto l = arena();
    Player p;
    p.pos = {40, 10};
    step(p, l, 240);
    CHECK(p.onGround && p.vel.y == 0);
    CHECK(jumpHeight(false) >= 48 && jumpHeight(false) <= 56);
    CHECK(jumpHeight(true) < jumpHeight(false) - 12);
    auto empty = l;
    empty.tiles[15] = std::string(30, '.');
    p = Player{};
    p.pos = {40, 50};
    p.coyoteTimer = Physics::COYOTE_TIME;
    step(p, empty, 6);
    Physics::step(p, {0, true, true, false}, empty, nullptr, dt);
    CHECK(p.vel.y < 0);
    p = Player{};
    p.pos = {40, 50};
    p.coyoteTimer = Physics::COYOTE_TIME;
    step(p, empty, 24);
    Physics::step(p, {0, true, true, false}, empty, nullptr, dt);
    CHECK(p.vel.y > 0);
    p = Player{};
    p.pos = {40, 216};
    p.vel.y = 200;
    Physics::step(p, {0, true, true, false}, l, nullptr, dt);
    step(p, l, 7);
    CHECK(p.vel.y < 0);
    for (int y = 0; y < 15; ++y)
        l.tiles[size_t(y)][8] = '#';
    p = Player{};
    p.pos = {115, 100};
    p.vel = {110, 420};
    step(p, l, 30, {1, false, false, false});
    CHECK(p.pos.x + 12 <= 128.1f);
    l = arena();
    l.tiles[10][4] = '=';
    p = Player{};
    p.pos = {66, 180};
    p.vel.y = -340;
    bool above = false;
    for (int i = 0; i < 100; ++i)
    {
        Physics::step(p, {}, l, nullptr, dt);
        above |= p.pos.y + 14 < 160;
    }
    CHECK(above);
    CHECK(p.onGround && std::abs(p.pos.y - 146) < 0.3f);
    Player bottom;
    bottom.pos = {60, 226};
    bottom.previous = bottom.pos;
    bottom.onGround = true;
    p = Player{};
    p.pos = {60, 200};
    for (int i = 0; i < 60; ++i)
        Physics::step(p, {}, l, &bottom, dt);
    CHECK(p.onGround && std::abs(p.pos.y - 212) < 0.3f);
    bottom.previous = bottom.pos;
    bottom.pos.x += 2;
    float before = p.pos.x;
    Physics::step(p, {}, l, &bottom, dt);
    CHECK(p.pos.x >= before + 1.9f);
    // A four-tile ledge is unreachable alone, but reachable from a teammate's head.
    l=arena();for(int y=11;y<15;++y)for(int x=13;x<17;++x)l.tiles[size_t(y)][size_t(x)]='#';
    p=Player{};p.pos={185,226};p.onGround=true;
    Physics::step(p,{1,true,true,false},l,nullptr,dt);step(p,l,85,{1,false,true,false});CHECK(p.pos.x<197);
    bottom=Player{};bottom.pos={185,226};bottom.previous=bottom.pos;bottom.onGround=true;
    p=Player{};p.pos={185,212};p.onGround=true;
    Physics::step(p,{1,true,true,false},l,&bottom,dt);
    for(int i=0;i<85;++i)Physics::step(p,{1,false,true,false},l,&bottom,dt);
    CHECK(p.pos.x>208&&p.pos.y<176);
    std::cout << "Physics scenarios passed\n";
    return 0;
}
