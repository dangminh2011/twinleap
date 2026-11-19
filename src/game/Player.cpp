#include "twinleap/game/Player.hpp"
namespace twinleap
{
Rect Player::bounds() const
{
    return {pos.x, pos.y, 12, 14};
}
} // namespace twinleap
