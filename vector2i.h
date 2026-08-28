#pragma once

struct Vector2i {
    int x, y;

    [[nodiscard]] constexpr Vector2 to_floats() const
    {
        return { static_cast<float>(x), static_cast<float>(y) };
    }
};
