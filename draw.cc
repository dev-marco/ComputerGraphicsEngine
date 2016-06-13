#include "draw.h"

namespace Engine {

    unsigned Draw::drawn = 0;

    std::stack<std::array<float_max_t, 16>> Draw::matrix_stack;
    Background *Draw::background;
    std::array<float_max_t, 16> Draw::matrix = {
        1.0, 0.0, 0.0, 0.0,
        0.0, 1.0, 0.0, 0.0,
        0.0, 0.0, 1.0, 0.0,
        0.0, 0.0, 0.0, 1.0
    };

};
