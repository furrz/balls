#pragma once
#include <raylib.h>
#include <vector>

#include "params.h"

namespace fonts {
    std::vector<Font> load(const params::ParamSet &params);
}
