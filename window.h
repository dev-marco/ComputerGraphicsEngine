#ifndef SRC_ENGINE_WINDOW_H_
#define SRC_ENGINE_WINDOW_H_

#include <queue>
#include <functional>
#include <map>
#include <unistd.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "shader.h"
#include "event.h"
#include "object.h"
#include "easing.h"
#include "texturepng.h"

namespace Engine {

    class Window {

        static std::map<GLFWwindow *, Window *> windows;

        GLFWwindow *window;
        Object object_root, gui_root;
        std::map<unsigned, std::tuple<std::function<bool()>, double, double, bool, bool>> timeouts;
        unsigned tick_counter = 0, timeout_counter = 1, pause_counter = 1;
        double start_time = 0, speed = 1.0;
        std::set<unsigned> paused;
        bool closed = false;
        std::queue<std::tuple<GLuint, double, double, std::valarray<double>>> textures;

        bool executeTimeout(std::map<unsigned, std::tuple<std::function<bool()>, double, double, bool, bool>>::iterator timeout);

    public:

        inline static Window *getInstance (GLFWwindow *_window) { return Window::windows[_window]; }

        inline Window (
            int width,
            int height,
            const char *title,
            GLFWmonitor *monitor = nullptr,
            GLFWwindow *share = nullptr
        ) : window(glfwCreateWindow(width, height, title, monitor, share)), start_time(glfwGetTime()) {
            glfwSetCursorPosCallback(this->window, Event::Event<Event::MouseMove>::trigger);
            glfwSetMouseButtonCallback(this->window, Event::Event<Event::MouseClick>::trigger);
            glfwSetKeyCallback(this->window, Event::Event<Event::Keyboard>::trigger);

            windows[this->window] = this;
        };

        inline ~Window (void) {
            windows.erase(this->window);
            glfwDestroyWindow(this->window);
        }

        void addTexture2D (const GLuint texture, const double width, const double height, const std::valarray<double> &position) {
            this->textures.push(std::forward_as_tuple(texture, width, height, position));
        }

        void drawNumber (const unsigned number, const double height, std::valarray<double> position) {

            const static GLuint textures_numbers[] = {
                loadPNG("images/numbers/0.png"),
                loadPNG("images/numbers/1.png"),
                loadPNG("images/numbers/2.png"),
                loadPNG("images/numbers/3.png"),
                loadPNG("images/numbers/4.png"),
                loadPNG("images/numbers/5.png"),
                loadPNG("images/numbers/6.png"),
                loadPNG("images/numbers/7.png"),
                loadPNG("images/numbers/8.png"),
                loadPNG("images/numbers/9.png")
            };

            const double width = height / 2.0;

            for (const char &c : std::to_string(number)) {
                addTexture2D(textures_numbers[c - '0'], width, height, position);
                position[0] += width / 1.5;
            }
        }

        void update(void);

        inline void addObject (Object *obj) { this->object_root.addChild(obj); }
        inline void addGUI (Object *gui) { this->gui_root.addChild(gui); }

        inline unsigned sync (unsigned fps = 60) {
            double frame_time = 1.0 / static_cast<double>(fps), now = glfwGetTime();
            if ((this->start_time + frame_time) > now) {
                usleep((this->start_time + frame_time - now) * 1000000.0);
            } else {
                fps = static_cast<unsigned>(round(1.0 / (now - this->start_time)));
            }
            this->start_time = glfwGetTime();
            return fps;
        }

        inline unsigned setTimeout (
            const std::function<bool()> &func,
            double interval,
            bool pauseable = false
        ) {
            if (interval > 0) {
                unsigned id = this->timeout_counter;
                this->timeout_counter++;
                if (this->isPaused() && pauseable) {
                    this->timeouts[id] = std::forward_as_tuple(func, interval, interval, true, pauseable);
                } else {
                    this->timeouts[id] = std::forward_as_tuple(func, glfwGetTime() + interval, interval, true, pauseable);
                }
                return id;
            }
            return 0;
        }

        inline void clearTimeout (unsigned id) {
            auto it = this->timeouts.find(id);
            if (it != this->timeouts.end()) {
                std::get<3>(it->second) = false;
            }
        }

        inline bool executeTimeout (unsigned id) {
            auto timeout = this->timeouts.find(id);
            if (timeout != this->timeouts.end()) {
                return this->executeTimeout(timeout);
            }
            return false;
        }

        unsigned animate (
            const std::function<bool(double)> &func,
            double total_time,
            unsigned total_steps = 0,
            std::function<double(double, double, double, double)> easing = Easing::Linear
        );

        void pause (unsigned &context);
        void unpause (unsigned &context);

        inline unsigned togglePaused (unsigned context) {
            if (this->isPaused()) {
                this->unpause(context);
            } else {
                this->pause(context);
            }
            return context;
        }

        inline void setShader (Shader::Program *shader) { this->object_root.setShader(shader); }

        inline void setSpeed (const double _speed) { this->speed = _speed; }
        inline double getSpeed (void) const { return this->speed; }

        inline void draw () {
            Shader::Program::useShader(this->object_root.getShader()), this->object_root.draw();
            Shader::Program::useShader(this->gui_root.getShader()), this->gui_root.draw();

            glEnable(GL_TEXTURE_2D);
            while (!this->textures.empty()) {

                GLuint texture;
                double width, height;
                std::valarray<double> position;

                std::tie(texture, width, height, position) = this->textures.front();

                this->textures.pop();

                glBindTexture(GL_TEXTURE_2D, texture);
                glBegin(GL_QUADS);
                glNormal3f(0.0, 0.0, 1.0);
                    glTexCoord2f(0, 0); glVertex3f(position[0], position[1], position[2]);
                    glTexCoord2f(0, 1); glVertex3f(position[0], position[1] + height, position[2]);
                    glTexCoord2f(1, 1); glVertex3f(position[0] + width, position[1] + height, position[2]);
                    glTexCoord2f(1, 0); glVertex3f(position[0] + width, position[1], position[2]);
                glEnd();
            }
            glDisable(GL_TEXTURE_2D);
        }

        inline void close (void) { this->closed = true; }

        inline void makeCurrentContext () const { glfwMakeContextCurrent(this->window); }
        inline bool shouldClose () const { return this->closed || glfwWindowShouldClose(this->window); }
        inline void swapBuffers () const { glfwSwapBuffers(this->window); }
        inline void getFramebufferSize (int &width, int &height) const { glfwGetFramebufferSize(this->window, &width, &height); }

        inline bool isPaused (void) const { return !this->paused.empty(); }

        inline GLFWwindow *get () const { return this->window; }

        inline operator bool () const { return this->window; }

        inline unsigned getTick () const { return this->tick_counter; }

        template <typename EventType>
        inline void event (typename EventType::FunctionType func, const std::string &id = "") {
            Event::Event<EventType>::add(this->window, func, id);
        }

        template <typename EventType>
        inline void eraseEvent (const std::string &id = "") {
            Event::Event<EventType>::erase(this->window, id);
        }

    };
};

#endif
