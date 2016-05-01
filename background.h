#ifndef SRC_BACKGROUND_H_
#define SRC_BACKGROUND_H_

#include <memory>
#include <iostream>
#include <GLFW/glfw3.h>
#include "color.h"

class Background {

    public:
        Background () {};
        virtual ~Background () {};
        virtual void apply () const {};
};

class BackgroundColor : virtual public Background {

    Color color;

public:
    inline BackgroundColor (const Color &_color) : color(_color) {};

    inline void setColor (const Color &_color) { this->color = _color; }

    inline void apply (void) const { this->color.apply(); }
};

#endif
