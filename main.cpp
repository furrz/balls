#include <algorithm>
#include <format>
#include <optional>
#include <raylib.h>
#include <raymath.h>
#include <vector>

#include "fonts.h"
#include "messages.h"
#include "params.h"
#include "phys.h"

bool resolve_collisions(const Vector2i win_size, const float circle_radius,
                        Vector2 &circle_pos, Vector2 &circle_velocity,
                        const float min_speed, const float restitution)
{
    const Vector2 corner = win_size.to_floats();

    const phys::Plane planes[] = {
            phys::Plane{{0, 0}, {1, 0}},
            phys::Plane{{0, 0}, {0, 1}},
            phys::Plane{corner, {-1, 0}},
            phys::Plane{corner, {0, -1}},
    };

    bool did_hit_wall = false;
    for (const auto &plane: planes) {
        did_hit_wall |= phys::circle_collide_and_bounce_plane(
                plane, circle_radius, circle_pos, circle_velocity);
    }

    if (did_hit_wall) {
        const auto speed = Vector2Length(circle_velocity);
        const auto dir = circle_velocity / speed;

        circle_velocity = dir * std::max(restitution * abs(speed), min_speed);
    }

    return did_hit_wall;
}

bool handle_circle_drag(const bool mouse_over_circle, Vector2 &circle_pos,
                        Vector2 &circle_velocity, bool circle_dragging,
                        const Vector2 mouse_delta, const float dt,
                        const float circle_drag_launch_speed)
{


    SetMouseCursor(mouse_over_circle ? MOUSE_CURSOR_POINTING_HAND
                                     : MOUSE_CURSOR_DEFAULT);

    if (mouse_over_circle && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        circle_dragging = true;
    }

    if (circle_dragging && IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
        circle_dragging = false;
        circle_velocity = mouse_delta / dt * circle_drag_launch_speed;
    }

    if (circle_dragging) {
        circle_pos += mouse_delta;
    }

    return circle_dragging;
}

struct RenderParams {
    Color bg_color;
    Color circle_color;
    Color circle_color_hover;
    Color target_color;

    static RenderParams load(const params::ParamSet &params)
    {
        return RenderParams{
                .bg_color = params.getc("render_bg_color"),
                .circle_color = params.getc("render_circle_color"),
                .circle_color_hover = params.getc("render_circle_color_hover"),
                .target_color = params.getc("render_target_color"),
        };
    }
};

struct CircleParams {
    Vector2 start_pos;
    float gravity;
    float radius;
    float hit_slop;
    float min_speed;
    float restitution;
    float drag_launch_speed;

    static CircleParams load(const params::ParamSet &params)
    {
        return CircleParams{
                .start_pos = params.getv("circle_pos"),
                .gravity = params.getf("circle_gravity"),
                .radius = params.getf("circle_radius"),
                .hit_slop = params.getf("circle_hit_slop"),
                .min_speed = params.getf("circle_min_speed"),
                .restitution = params.getf("circle_restitution"),
                .drag_launch_speed = params.getf("circle_drag_launch_speed"),
        };
    }
};

struct WindowParams {
    const char *title;
    Vector2i size;
    int target_fps;
    float max_dt;

    static WindowParams load(const params::ParamSet &params)
    {
        return WindowParams{
                .title = params.gets("win_title"),
                .size = params.getvi("win_size"),
                .target_fps = params.geti("win_target_fps"),
                .max_dt = 1.0f / params.getf("win_min_fps"),
        };
    }
};

struct TargetParams {
    int count;
    int radius_min;
    int radius_max;
    float spawn_interval;
    float growth_speed;
    Vector2 growth_ctl_1;
    Vector2 growth_ctl_2;

    static TargetParams load(const params::ParamSet &params)
    {
        return TargetParams{
                .count = params.geti("target_count"),
                .radius_min = params.geti("target_radius_min"),
                .radius_max = params.geti("target_radius_max"),
                .spawn_interval = params.getf("target_spawn_interval"),
                .growth_speed = params.getf("target_growth_speed"),
                .growth_ctl_1 = params.getv("target_growth_ctl_1"),
                .growth_ctl_2 = params.getv("target_growth_ctl_2"),
        };
    }
};

struct TargetList {
    std::vector<phys::Circle> circles;
    std::vector<float> spawn_times;

    void emplace_back(phys::Circle circle, float spawn_time)
    {
        circles.emplace_back(circle);
        spawn_times.emplace_back(spawn_time);
    }

    void remove(const size_t index)
    {
        circles[index] = circles.back();
        circles.pop_back();
        spawn_times[index] = spawn_times.back();
        spawn_times.pop_back();
    }

    void reserve(const size_t size)
    {
        circles.reserve(size);
        spawn_times.reserve(size);
    }

    [[nodiscard]] size_t size() const { return circles.size(); }

    static TargetList with_capacity(const size_t size)
    {
        TargetList list{};
        list.reserve(size);
        return std::move(list);
    }
};

struct GameSounds {
    Sound bump;
    Sound pop_1;
    Sound pop_2;

    static GameSounds prepare(const params::ParamSet &params)
    {
        SetMasterVolume(params.getf("sfx_master_volume"));

        const Sound bump_sfx = LoadSound(params.gets("sfx_bump_path"));

        const char *pop_path = params.gets("sfx_pop_path");
        const Sound pop_sfx = LoadSound(pop_path);
        const Sound pop_sfx_2 = LoadSound(pop_path);
        SetSoundPitch(pop_sfx_2, params.getf("sfx_pop_secondary_pitch"));

        return GameSounds{
                .bump = bump_sfx, .pop_1 = pop_sfx, .pop_2 = pop_sfx_2};
    }
};

void draw_targets(const RenderParams &render_params,
                  const TargetParams &target_params,
                  const TargetList &targets,
                  const float time)
{
    for (int i = 0; i < targets.size(); i++) {
        const auto &target = targets.circles[i];
        const auto &spawn_time = targets.spawn_times[i];
        const float scale_t = std::clamp(
                (time - spawn_time) * target_params.growth_speed, 0.0f, 1.0f);
        const float scale_factor =
                GetSplinePointBezierCubic({0, 0}, target_params.growth_ctl_1,
                                          target_params.growth_ctl_2, {1, 1},
                                          scale_t)
                        .y;
        DrawCircleV(target.point, target.radius * scale_factor,
                    render_params.target_color);
    }
}

phys::Circle gen_random_target_circle(const TargetParams &params,
                                      const Vector2i win_size)
{
    const auto radius = GetRandomValue(params.radius_min, params.radius_max);
    const auto point = Vector2{
            .x = static_cast<float>(
                    GetRandomValue(radius, win_size.x - radius)),
            .y = static_cast<float>(
                    GetRandomValue(radius, win_size.y - radius)),
    };

    return {.point = point, .radius = static_cast<float>(radius)};
}

int main()
{
    // MUST live for the program lifetime to keep strings intact!
    const auto params = params::load_params_file("res/params.cfg");

    const auto window_params = WindowParams::load(params);
    SetConfigFlags(FLAG_WINDOW_HIGHDPI | FLAG_MSAA_4X_HINT);
    InitWindow(window_params.size.x, window_params.size.y, window_params.title);
    InitAudioDevice();
    SetTargetFPS(window_params.target_fps);

    const auto sfx = GameSounds::prepare(params);
    const auto render_params = RenderParams::load(params);
    const auto circle_params = CircleParams::load(params);
    const auto target_params = TargetParams::load(params);
    const auto fonts = fonts::load(params);
    const auto messages = messages::load(params, window_params.size, fonts);

    Vector2 circle_pos = circle_params.start_pos;
    Vector2 circle_velocity = {};
    bool circle_dragging = false;

    float target_spawn_countdown = target_params.spawn_interval;
    auto targets = TargetList::with_capacity(target_params.count);


    while (!WindowShouldClose()) {
        const Vector2 mouse_pos = GetMousePosition();
        const Vector2 mouse_delta = GetMouseDelta();
        const auto time = static_cast<float>(GetTime());
        const float dt = std::min(GetFrameTime(), window_params.max_dt);

        const bool mouse_over_circle = CheckCollisionPointCircle(
                mouse_pos, circle_pos,
                circle_params.radius + circle_params.hit_slop);

        circle_dragging = handle_circle_drag(
                mouse_over_circle, circle_pos, circle_velocity, circle_dragging,
                mouse_delta, dt, circle_params.drag_launch_speed);

        if (auto hit_target = phys::circle_find_overlap(
                    phys::Circle{.point = circle_pos,
                                 .radius = circle_params.radius},
                    targets.circles)) {
            if (!IsSoundPlaying(sfx.pop_1))
                PlaySound(sfx.pop_1);
            else
                PlaySound(sfx.pop_2);

            targets.remove(*hit_target);
        }

        if (targets.size() < target_params.count) {
            target_spawn_countdown -= dt;
            if (target_spawn_countdown <= 0) {
                target_spawn_countdown = target_params.spawn_interval;

                targets.emplace_back(gen_random_target_circle(
                                             target_params, window_params.size),
                                     time);
            }
        }

        if (!circle_dragging) {
            circle_pos += circle_velocity * dt;
            circle_velocity.y += circle_params.gravity * dt;

            const bool did_hit_wall = resolve_collisions(
                    window_params.size, circle_params.radius, circle_pos,
                    circle_velocity, circle_params.min_speed,
                    circle_params.restitution);

            if (did_hit_wall)
                PlaySound(sfx.bump);
        }

        BeginDrawing();
        ClearBackground(render_params.bg_color);

        draw_targets(render_params, target_params, targets, time);

        const bool darken_circle = mouse_over_circle && !circle_dragging;
        DrawCircleV(circle_pos, circle_params.radius,
                    darken_circle ? render_params.circle_color_hover
                                  : render_params.circle_color);

        messages::draw(messages, fonts);

        EndDrawing();
    }

    CloseAudioDevice();
    CloseWindow();
    return 0;
}
