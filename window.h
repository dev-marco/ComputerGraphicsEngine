#ifndef SRC_WINDOW_H_
#define SRC_WINDOW_H_

#include <functional>
#include <map>
#include <unistd.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "shader.h"
#include "event.h"
#include "object.h"
#include "easing.h"

class Window {

    GLFWwindow *window;
    Object root;
    std::map<unsigned, std::tuple<std::function<bool()>, double, double>> timeouts;
    unsigned tick_counter = 0;

    inline bool executeTimeout (std::map<unsigned, std::tuple<std::function<bool()>, double, double>>::iterator timeout) {
        if (std::get<0>(timeout->second)()) {
            std::get<1>(timeout->second) += std::get<2>(timeout->second);
            return true;
        }
        this->timeouts.erase(timeout);
        return false;
    }

public:

    inline Window (int width, int height, const char *title, GLFWmonitor *monitor, GLFWwindow *share) :
        window(glfwCreateWindow(width, height, title, monitor, share)) {
        glfwSetCursorPosCallback(this->window, Event::Event<Event::MouseMove>::trigger);
    };

    void update () {

        double now = glfwGetTime();
        auto timeout = this->timeouts.begin();

        this->root.detectCollisions();
        this->root.update(now, this->tick_counter);

        while (timeout != this->timeouts.end()) {

            auto next = std::next(timeout, 1);

            if (std::get<1>(timeout->second) <= now) {
                this->executeTimeout(timeout);
            }

            timeout = next;
        }

        this->tick_counter++;
    }

    inline void addObject (Object *obj) {
        this->root.addChild(obj);
    }

    inline unsigned sync (unsigned fps = 60) {
        static double start_time = 0;
        double frame_time = 1.0 / static_cast<double>(fps), now = glfwGetTime();
        if ((start_time + frame_time) > now) {
            usleep((start_time + frame_time - now) * 1000000.0);
        } else {
            fps = static_cast<unsigned>(round(1.0 / (now - start_time)));
        }
        start_time = glfwGetTime();
        return fps;
    }

    inline unsigned setTimeout (const std::function<bool()> &func, double interval) {

        static unsigned timeout_counter = 0;

        double now = glfwGetTime();

        unsigned id = timeout_counter;
        timeout_counter++;

        this->timeouts[id] = std::forward_as_tuple(func, now + interval, interval);

        return id;
    }

    inline void clearTimeout (unsigned id) {
        this->timeouts.erase(id);
    }

    inline bool executeTimeout (unsigned id) {
        auto timeout = this->timeouts.find(id);
        if (timeout != this->timeouts.end()) {
            return this->executeTimeout(timeout);
        }
        return false;
    }

    inline unsigned animate (
        const std::function<bool(double)> &func,
        double total_time,
        unsigned total_steps = 0,
        std::function<double(double, double, double, double)> easing = Easing::Linear
    ) {

        double delta, start_time = glfwGetTime();

        if (total_steps == 0) {
            total_steps = ceil(total_time / 0.01);
        }

        delta = 1.0 / static_cast<double>(total_steps);

        return this->setTimeout ([ delta, func, easing, start_time, total_time ] () -> bool {
            double now = glfwGetTime();
            if (now < (total_time + start_time)) {
                return func(easing(now - start_time, 0.0, 1.0, total_time));
            }
            func(1.0);
            return false;
        }, total_time * delta);
    }

    inline void completeAnimation (unsigned id) {
        auto timeout = this->timeouts.find(id);
        if (timeout != this->timeouts.end()) {
            while(this->executeTimeout(timeout));
        }
    }

    inline void setShader (const Shader::Program *shader) {
        this->root.setShader(shader);
    }

    inline void draw () const {
        const Shader::Program *shader = this->root.getShader();
        if (shader != nullptr) {
            this->root.getShader()->use();
        }
        this->root.draw();
    }

    inline void makeCurrentContext () const {
        glfwMakeContextCurrent(this->window);
    }

    inline bool shouldClose () const {
        return glfwWindowShouldClose(this->window);
    }

    inline void swapBuffers () const {
        glfwSwapBuffers(this->window);
    }

    inline void getFramebufferSize (int &width, int &height) const {
        glfwGetFramebufferSize(this->window, &width, &height);
    }

    inline GLFWwindow *get () const {
        return this->window;
    }

    inline operator bool () const {
        return this->window;
    }

    inline unsigned getTick () const {
        return this->tick_counter;
    }

    template <typename EventType>
    inline void event (typename EventType::FunctionType func, const std::string &id = "") {
        Event::Event<EventType>::add(this->window, func, id);
    }

};

#endif
