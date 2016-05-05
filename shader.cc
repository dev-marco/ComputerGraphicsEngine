#include "shader.h"

namespace Engine {
    namespace Shader {
        Program *Program::current_shader = nullptr;
        std::stack<Program *> Program::programs;
    };
};
