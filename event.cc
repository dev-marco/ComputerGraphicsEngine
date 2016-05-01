#include "event.h"

double Event::MouseMove::posx = 0.0;
double Event::MouseMove::posy = 0.0;

std::unordered_map<GLFWwindow *, std::list<std::tuple<Event::MouseMove::FunctionType, std::string, unsigned, bool>>> Event::MouseMove::trigger_list;

template <typename EventType>
void Event::Event<EventType>::erase (GLFWwindow *window, const FunctionType &func) {
    for (const auto &ev : EventType::trigger_list[window]) {
        if (std::get<0>(ev) == func) {
            std::get<3>(ev) = true;
        }
    }
}

template <typename EventType>
void Event::Event<EventType>::erase (GLFWwindow *window, const std::string &id) {
    if (id != "") {
        for (const auto &ev : EventType::trigger_list[window]) {
            if (std::get<1>(ev) == id) {
                std::get<3>(ev) = true;
            }
        }
    }
}
