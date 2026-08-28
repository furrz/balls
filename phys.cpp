#include "phys.h"
#include <raymath.h>

namespace phys {
    std::optional<Collision> circle_collide_plane(const Plane &plane,
                                                  const Circle &circle)
    {
        const float distance =
                Vector2DotProduct(plane.normal, circle.point - plane.point)
                - circle.radius;

        if (distance >= 0)
            return {};

        return Collision{
                .penetration = -distance,
                .normal = plane.normal,
        };
    }

    bool circle_collide_and_bounce_plane(const Plane &plane, const float radius,
                                         Vector2 &pos, Vector2 &velocity)
    {
        const auto coll = circle_collide_plane(plane, Circle{pos, radius});
        if (!coll)
            return false;

        pos += coll->normal * coll->penetration;
        velocity = Vector2Reflect(velocity, coll->normal);
        return true;
    }

} // namespace phys
