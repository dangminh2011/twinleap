#include "twinleap/game/Camera.hpp"
#include <cmath>
namespace twinleap
{
void Camera::update(const std::array<Player, 2> &players, int width, int height, float dt)
{
    Vec2 target{};
    int alive = 0;
    for (const auto &p : players)
        if (p.status != Player::Status::Dead)
        {
            target.x += p.pos.x + 6;
            target.y += p.pos.y + 7;
            ++alive;
        }
    if (!alive)
        return;
    auto bound = [](float value, float size, float screen) {
        return size < screen ? (size - screen) / 2 : std::clamp(value, 0.f, size - screen);
    };
    target.x = bound(target.x / float(alive) - 240, float(width * 16), 480);
    target.y = bound(target.y / float(alive) - 135, float(height * 16), 270);
    float smooth = 1 - std::exp(-8 * dt);
    position.x += (target.x - position.x) * smooth;
    position.y += (target.y - position.y) * smooth;
}
} // namespace twinleap
