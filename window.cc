#include "window.h"

namespace Engine {

    std::map<GLFWwindow *, Window *> Window::windows;

    bool Window::executeTimeout (std::map<unsigned, std::tuple<std::function<bool()>, double, double, bool, bool>>::iterator timeout) {
        if (std::get<3>(timeout->second)) {
            if (!(std::get<4>(timeout->second) && this->isPaused())) {
                if (std::get<0>(timeout->second)()) {
                    if (!(std::get<4>(timeout->second) && this->isPaused())) {
                        std::get<1>(timeout->second) = std::get<2>(timeout->second);
                    } else {
                        std::get<1>(timeout->second) += std::get<2>(timeout->second);
                    }
                    return true;
                }
            } else {
                return true;
            }
        }
        this->timeouts.erase(timeout);
        return false;
    }

    void Window::pause (unsigned &context) {
        this->unpause(context);
        context = this->pause_counter++;
        if (!this->isPaused()) {
            double now = glfwGetTime();
            for (auto &timeout : this->timeouts) {
                if (std::get<4>(timeout.second)) {
                    std::get<1>(timeout.second) -= now;
                }
            }
        }
        this->paused.insert(context);
    }

    void Window::unpause (unsigned &context) {
        if (!this->paused.empty()) {
            this->paused.erase(context);
            context = 0;
            if (this->paused.empty()) {
                double now = glfwGetTime();
                for (auto &timeout : this->timeouts) {
                    if (std::get<4>(timeout.second)) {
                        std::get<1>(timeout.second) += now;
                    }
                }
            }
        }
    }

    void Window::update (void) {

        static double first_time = 0;
        double now = glfwGetTime(), delta_time = (now - first_time) * speed;

        first_time = now;

        if (!this->isPaused()) {

            this->object_root.update(now, delta_time, this->tick_counter, true);
            this->gui_root.update(now, delta_time, this->tick_counter, false);

            this->tick_counter++;
        }

        auto timeout = this->timeouts.begin();
        while (timeout != this->timeouts.end()) {

            auto next = std::next(timeout, 1);

            if (std::get<1>(timeout->second) <= now) {
                this->executeTimeout(timeout);
            }

            timeout = next;
        }
    }

    unsigned Window::animate (
        const std::function<bool(double)> &func,
        double total_time,
        unsigned total_steps,
        std::function<double(double, double, double, double)> easing
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
};
