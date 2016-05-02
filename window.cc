#include "window.h"

bool Window::executeTimeout (std::map<unsigned, std::tuple<std::function<bool()>, double, double>>::iterator timeout) {
    if (std::get<0>(timeout->second)()) {
        std::get<1>(timeout->second) += std::get<2>(timeout->second);
        return true;
    }
    this->timeouts.erase(timeout);
    return false;
}

void Window::update (void) {

    static double first_time = 0;
    double now = glfwGetTime(), delta_time = now - first_time;
    auto timeout = this->timeouts.begin();

    this->object_root.update(now, delta_time, this->tick_counter, true);
    this->gui_root.update(now, delta_time, this->tick_counter, false);

    first_time = now;

    while (timeout != this->timeouts.end()) {

        auto next = std::next(timeout, 1);

        if (std::get<1>(timeout->second) <= now) {
            this->executeTimeout(timeout);
        }

        timeout = next;
    }

    this->tick_counter++;
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
