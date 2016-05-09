#include "event.h"

namespace Engine {

    namespace Event {
        double MouseMove::posx = 0.0;
        double MouseMove::posy = 0.0;

        std::unordered_map<GLFWwindow *, std::list<std::tuple<MouseMove::FunctionType, std::string, unsigned, bool>>> MouseMove::trigger_list;
        std::unordered_map<GLFWwindow *, std::list<std::tuple<Keyboard::FunctionType, std::string, unsigned, bool>>> Keyboard::trigger_list;
    };

};
