#include "fonts.h"

#include <format>

std::vector<Font> fonts::load(const params::ParamSet &params)
{
    const auto dpi_scale =
            std::max({GetWindowScaleDPI().x, GetWindowScaleDPI().y, 1.0f});

    const int font_count = params.geti("font_count");
    std::vector<Font> fonts;
    fonts.reserve(font_count);

    for (int i = 0; i < font_count; i++) {
        fonts.emplace_back(LoadFontEx(
                params.gets(std::format("font{}_path", i).c_str()),
                static_cast<int>(
                        ceilf(params.getf(std::format("font{}_size", i).c_str())
                              * dpi_scale)),
                nullptr, 0));
    }
    return fonts;
}