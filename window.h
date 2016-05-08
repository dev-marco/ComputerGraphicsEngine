#ifndef SRC_ENGINE_WINDOW_H_
#define SRC_ENGINE_WINDOW_H_

#include <functional>
#include <map>
#include <unistd.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "shader.h"
#include "event.h"
#include "object.h"
#include "easing.h"

namespace Engine {
    class Window {

        GLFWwindow *window;
        Object object_root, gui_root;
        std::map<unsigned, std::tuple<std::function<bool()>, double, double, bool, bool>> timeouts;
        unsigned tick_counter = 0, timeout_counter = 1;
        double start_time = 0;
        bool paused = false;

        bool executeTimeout(std::map<unsigned, std::tuple<std::function<bool()>, double, double, bool, bool>>::iterator timeout);

    public:

        inline Window (
            int width,
            int height,
            const char *title,
            GLFWmonitor *monitor = nullptr,
            GLFWwindow *share = nullptr
        ) : window(glfwCreateWindow(width, height, title, monitor, share)) {
            glfwSetCursorPosCallback(this->window, Event::Event<Event::MouseMove>::trigger);
        };

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

        void pause (void);
        void unpause (void);

        inline void setShader (Shader::Program *shader) { this->object_root.setShader(shader); }

        inline void draw () const {
            Shader::Program::useShader(this->object_root.getShader()), this->object_root.draw();
            Shader::Program::useShader(this->gui_root.getShader()), this->gui_root.draw();
        }

        inline void makeCurrentContext () const { glfwMakeContextCurrent(this->window); }
        inline bool shouldClose () const { return glfwWindowShouldClose(this->window); }
        inline void swapBuffers () const { glfwSwapBuffers(this->window); }
        inline void getFramebufferSize (int &width, int &height) const { glfwGetFramebufferSize(this->window, &width, &height); }

        inline bool isPaused (void) const { return this->paused; }

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
