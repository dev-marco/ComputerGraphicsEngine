#include "event.h"

namespace Engine {

    namespace Event {
        float_max_t MouseMove::posx = 0.0;
        float_max_t MouseMove::posy = 0.0;

        std::unordered_map<GLFWwindow *, std::list<std::tuple<MouseMove::FunctionType, std::string, unsigned, bool>>> MouseMove::trigger_list;
        std::unordered_map<GLFWwindow *, std::list<std::tuple<MouseClick::FunctionType, std::string, unsigned, bool>>> MouseClick::trigger_list;
        std::unordered_map<GLFWwindow *, std::list<std::tuple<Keyboard::FunctionType, std::string, unsigned, bool>>> Keyboard::trigger_list;
    };

};
