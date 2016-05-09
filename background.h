#ifndef SRC_ENGINE_BACKGROUND_H_
#define SRC_ENGINE_BACKGROUND_H_

#include <memory>
#include <iostream>
#include <GLFW/glfw3.h>
#include "color.h"

namespace Engine {

    class Background {

        public:
            Background () {};
            virtual ~Background () {};
            virtual void apply () const {};
    };

    class BackgroundColor : public Background {

        Color color;

    public:
        inline BackgroundColor () : color(Color::rgb(0, 0, 0)) {};
        inline BackgroundColor (Color _color) : color(_color) {};

        inline void setColor (Color &_color) { this->color = _color; }

        inline void setR (unsigned char _r) { this->color.setR(_r); }
        inline void setG (unsigned char _g) { this->color.setR(_g); }
        inline void setB (unsigned char _b) { this->color.setR(_b); }
        inline void setA (double _a) { this->color.setA(_a); }

        inline void apply (void) const { this->color.apply(); }
    };
};

#endif
