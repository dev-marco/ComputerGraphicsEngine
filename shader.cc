#include "shader.h"

Shader::Program *Shader::Program::current_shader = nullptr;
std::stack<Shader::Program *> Shader::Program::programs;
