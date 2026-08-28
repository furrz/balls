#pragma once
#include <raylib.h>
#include <string>
#include <unordered_map>
#include "vector2i.h"

namespace params {

    struct ParamSet {
        std::unordered_map<std::string, std::string> params;
        const char * gets(const char *key) const;
        float getf(const char *key) const;
        int geti(const char *key) const;
        Color getc(const char *key) const;
        Vector2 getv(const char *key) const;
        Vector2i getvi(const char *key) const;
    };

    ParamSet load_params_file(const char *path);



}