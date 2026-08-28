#include "params.h"

#include <bit>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <ranges>
#include <sstream>

const char *params::ParamSet::gets(const char *key) const
{
    const auto found = params.find(key);
    if (found == params.end()) {
        std::cerr << "Missing param: " << key << std::endl;
        exit(1);
    }
    return found->second.c_str();
}

float params::ParamSet::getf(const char *key) const { return atof(gets(key)); }

int params::ParamSet::geti(const char *key) const { return atoi(gets(key)); }

Color params::ParamSet::getc(const char *key) const
{
    return std::bit_cast<Color>(std::byteswap(
            static_cast<uint32_t>(std::stoul(gets(key), nullptr, 16))
            << 8
            | 0xFF));
}

template<typename T>
T from_string_view(const std::string_view sv)
{
    T v;
    std::from_chars(sv.data(), sv.data() + sv.length(), v);
    return v;
}

Vector2 params::ParamSet::getv(const char *key) const
{
    namespace v = std::views;
    const auto value = gets(key);
    auto parts = std::string_view(value)
                 | v::split(' ')
                 | v::transform([](auto r) { return std::string_view(r); })
                 | v::transform(from_string_view<float>)
                 | v::adjacent<2>;
    const auto [xv, yv] = *parts.begin();
    return {xv, yv};
}

Vector2i params::ParamSet::getvi(const char *key) const
{
    namespace v = std::views;
    const auto value = gets(key);
    auto parts = std::string_view(value)
                 | v::split(' ')
                 | v::transform([](auto r) { return std::string_view(r); })
                 | v::transform(from_string_view<int>)
                 | v::adjacent<2>;
    const auto [xv, yv] = *parts.begin();
    return {xv, yv};
}

params::ParamSet params::load_params_file(const char *path)
{
    std::ifstream in(path);
    if (!in.is_open()) {
        std::cerr << "Failed to open params file: " << path << std::endl;
        exit(1);
    }

    ParamSet result;

    std::string line;
    while (std::getline(in, line)) {
        if (line.empty() || line.starts_with('#'))
            continue;
        std::stringstream iss(line);
        std::string key, value;
        iss >> key >> std::quoted(value);
        result.params[key] = value;
    }

    return result;
}
