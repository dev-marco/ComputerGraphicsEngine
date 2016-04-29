#ifndef SRC_COLOR_H_
#define SRC_COLOR_H_

#include <GLFW/glfw3.h>

class Color {

    double r, g, b, a;

public:

    Color (double _r, double _g, double _b, double _a = 1.0) :
        r(_r), g(_g), b(_b), a(_a) {};

    static Color rgba (unsigned char _r, unsigned char _g, unsigned char _b, double _a) {
        return Color(_r / 255.0, _g / 255.0, _b / 255.0, _a);
    }

    static Color rgb (unsigned char _r, unsigned char _g, unsigned char _b) {
        return Color(_r / 255.0, _g / 255.0, _b / 255.0, 1.0);
    }

    void apply () const {
        glColor4d(this->r, this->g, this->b, this->a);
    }

};

#endif
