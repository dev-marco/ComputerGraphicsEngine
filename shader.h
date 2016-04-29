#ifndef SRC_SHADER_H_
#define SRC_SHADER_H_

#include <string>
#include <vector>
#include <stack>
#include <set>
#include <map>
#include <iostream>
#include <functional>
#include <GL/glew.h>

namespace Shader {

    static const class Program *current_shader = nullptr;

    class Program {

        GLuint prog = 0;
        std::map<GLuint, std::set<GLuint>> shaders {
            std::make_pair(GL_VERTEX_SHADER, std::set<GLuint>()),
            std::make_pair(GL_FRAGMENT_SHADER, std::set<GLuint>()),
            std::make_pair(GL_GEOMETRY_SHADER, std::set<GLuint>()),
            std::make_pair(GL_COMPUTE_SHADER, std::set<GLuint>()),
            std::make_pair(GL_TESS_CONTROL_SHADER, std::set<GLuint>()),
            std::make_pair(GL_TESS_EVALUATION_SHADER, std::set<GLuint>())
        };
        bool linked = false, multiple = false;
        std::function<void(const Shader::Program *)> before_use, after_use;

        static GLuint compile (GLuint type, const std::vector<std::string> &src) {
            unsigned size = src.size();
            GLuint result;
            const char **source = new const char*[size];
            for (unsigned i = 0; i < size; i++) {
                source[i] = src[i].c_str();
            }
            result = Program::compile(type, source, size);
            delete[] source;
            return result;
        }

        static GLuint compile (GLuint type, const char **src, unsigned size) {
            GLint compiled;
            GLuint shader = glCreateShader(type);
            glShaderSource(shader, size, src, nullptr);
            glCompileShader(shader);
            glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
            if (compiled) {
                return shader;
            } else {
                GLint length;
                std::string shader_error;
                glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
                shader_error.resize(length);
                glGetShaderInfoLog(shader, length, &length, &shader_error[0]);
                throw shader_error;
                return GL_FALSE;
            }
        }

        GLuint attachShader (GLuint shader, GLuint type) {
            if (!this->linked && shader != GL_FALSE && this->shaders.find(type) != this->shaders.end()) {
                if (this->multiple || this->shaders[type].size() == 0) {
                    this->shaders[type].insert(shader);
                    glAttachShader(this->prog, shader);
                    return shader;
                }
                throw std::string("This program does not support more than one shade of the same type. Initialize it passing true as the parameter.");
            }
            return GL_FALSE;
        }

    public:

        inline Program (bool _multiple = false) : prog(glCreateProgram()), multiple(_multiple) {}

        inline ~Program (void) { glDeleteProgram(this->prog); }

        GLuint addCompiledShader (GLuint shader) {
            GLint compiled;
            glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
            if (compiled) {
                GLint type;
                glGetShaderiv(shader, GL_SHADER_TYPE, &type);
                return this->attachShader(shader, type);
            }
            return GL_FALSE;
        }

        inline GLuint attachVertexShader (const std::vector<std::string> &src) {
            return this->attachShader(Program::compile(GL_VERTEX_SHADER, src), GL_VERTEX_SHADER);
        }

        inline GLuint attachFragmentShader (const std::vector<std::string> &src) {
            return this->attachShader(Program::compile(GL_FRAGMENT_SHADER, src), GL_FRAGMENT_SHADER);
        }

        inline GLuint attachGeometryShader (const std::vector<std::string> &src) {
            return this->attachShader(Program::compile(GL_GEOMETRY_SHADER, src), GL_GEOMETRY_SHADER);
        }

        inline GLuint attachComputeShader (const std::vector<std::string> &src) {
            return this->attachShader(Program::compile(GL_COMPUTE_SHADER, src), GL_COMPUTE_SHADER);
        }

        inline GLuint attachTesselationControlShader (const std::vector<std::string> &src) {
            return this->attachShader(Program::compile(GL_COMPUTE_SHADER, src), GL_TESS_CONTROL_SHADER);
        }

        inline GLuint attachTesselationEvalShader (const std::vector<std::string> &src) {
            return this->attachShader(Program::compile(GL_COMPUTE_SHADER, src), GL_TESS_EVALUATION_SHADER);
        }

        inline void detachShader (GLuint shader) {
            GLint type;
            glGetShaderiv(shader, GL_SHADER_TYPE, &type);
            this->shaders[type].erase(shader);
        }

        inline void link (void) {
            if (*this) {
                glLinkProgram(this->prog);
                this->linked = true;
            } else {
                throw std::string("A program should have at least one GL_VERTEX_SHADER and GL_FRAGMENT_SHADER to work.");
            }
        }

        inline bool use (void) const {
            if (*this) {
                Shader::current_shader = this;
                if (this->before_use) {
                    this->before_use(this);
                }
                glUseProgram(this->prog);
                if (this->after_use) {
                    this->after_use(this);
                }
                return true;
            }
            return false;
        }

        inline void onBeforeUse (const std::function<void(const Shader::Program *)> &func) {
            this->before_use = func;
        }

        inline void onAfterUse (const std::function<void(const Shader::Program *)> &func) {
            this->after_use = func;
        }

        inline GLuint getProgramID (void) const { return this->prog; }

        GLint getUniformLocationARB (const std::string &name) const {
            return glGetUniformLocationARB(this->prog, name.c_str());
        }

        inline void operator() (void) const { this->use(); }

        inline bool operator != (const Shader::Program& other) const {
            return this->prog != other.prog;
        }

        inline operator GLuint (void) const { return this->getProgramID(); }

        inline operator bool () const { return this->shaders.at(GL_VERTEX_SHADER).size() && this->shaders.at(GL_FRAGMENT_SHADER).size(); }
    };
}

#endif
