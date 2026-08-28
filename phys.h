#pragma once
#include <optional>
#include <raylib.h>
#include <span>

namespace phys {
    struct Circle {
        Vector2 point;
        float radius;
    };

    struct Plane {
        Vector2 point;
        Vector2 normal;
    };

    struct Collision {
        float penetration;
        Vector2 normal;
    };

    std::optional<Collision> circle_collide_plane(const Plane &plane,
                                                  const Circle &circle);

    bool circle_collide_and_bounce_plane(const Plane &plane, float radius,
                                         Vector2 &pos, Vector2 &velocity);

    template<typename Range>
    std::optional<size_t> circle_find_overlap(const Circle &main_circle,
                                              const Range &circle_range) {

        const auto it = std::ranges::find_if(
                circle_range, [&](const Circle &other_circle) {
                    return CheckCollisionCircles(
                            main_circle.point, main_circle.radius,
                            other_circle.point, other_circle.radius);
                });

        if (it != circle_range.end())
            return std::distance(circle_range.begin(), it);
        return {};
    }

} // namespace phys
