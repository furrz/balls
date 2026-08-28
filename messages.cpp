#include "messages.h"

#include <format>
std::vector<messages::Message> messages::load(const params::ParamSet &params,
                                              Vector2i win_size,
                                              const std::vector<Font> &fonts)
{
    const int message_count = params.geti("message_count");
    std::vector<Message> messages;
    messages.reserve(message_count);

    const auto win_size_f = win_size.to_floats();

    for (int i = 0; i < message_count; i++) {
        Message message{
                .text = params.gets(std::format("message{}_text", i).c_str()),
                .font = params.geti(std::format("message{}_font", i).c_str()),
                .color = params.getc(std::format("message{}_color", i).c_str()),
                .pos = params.getv(std::format("message{}_pos", i).c_str())};

        const auto size = MeasureTextEx(
                fonts[message.font], message.text,
                static_cast<float>(fonts[message.font].baseSize), 1.0f);

        if (message.pos.x < 0)
            message.pos.x += win_size_f.x - size.x;
        if (message.pos.y < 0)
            message.pos.y += win_size_f.y - size.y;

        messages.emplace_back(message);
    }

    return messages;
}
void messages::draw(const std::vector<Message> &messages,
                    const std::vector<Font> &fonts)
{
    for (const auto &message: messages) {
        DrawTextEx(fonts[message.font], message.text, message.pos,
                   static_cast<float>(fonts[message.font].baseSize), 1.0f,
                   message.color);
    }
}
