#pragma once
#include <raylib.h>
#include <vector>

#include "params.h"

namespace messages {

    struct Message {
        const char *text;
        int font;
        Color color;
        Vector2 pos;
    };

    std::vector<Message> load(const params::ParamSet &params, Vector2i win_size,
                              const std::vector<Font> &fonts);

    void draw(const std::vector<Message> &messages,
              const std::vector<Font> &fonts);

} // namespace messages
