#include "event.h"

namespace Engine {
    double Event::MouseMove::posx = 0.0;
    double Event::MouseMove::posy = 0.0;

    std::unordered_map<GLFWwindow *, std::list<std::tuple<Event::MouseMove::FunctionType, std::string, unsigned, bool>>> Event::MouseMove::trigger_list;
};
