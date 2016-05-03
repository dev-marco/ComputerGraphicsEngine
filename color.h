#ifndef SRC_ENGINE_COLOR_H_
#define SRC_ENGINE_COLOR_H_

#include <string>
#include <GLFW/glfw3.h>

class Color {

    double r, g, b, a;

public:

    inline Color (double _r, double _g, double _b, double _a = 1.0) :
        r(_r), g(_g), b(_b), a(_a) {};

    inline static Color rgba (unsigned char _r, unsigned char _g, unsigned char _b, double _a) {
        return Color(_r / 255.0, _g / 255.0, _b / 255.0, _a);
    }

    inline static Color rgb (unsigned char _r, unsigned char _g, unsigned char _b) {
        return Color(_r / 255.0, _g / 255.0, _b / 255.0, 1.0);
    }

    inline static Color rgb (unsigned value) {
        return rgb((value >> 16) & 255, (value >> 8) & 255, value & 255);
    }

    inline static Color rgba (unsigned value) {
        return rgba((value >> 16) & 255, (value >> 8) & 255, value & 255, static_cast<double>((value >> 24) & 255) / 255.0);
    }

    inline static Color hex (std::string str) {
        if (str[0] == '#') {
            str = str.substr(1);
        }
        return rgb(stoul(str, nullptr, 16));
    }

    inline void setR (unsigned char _r) { this->r = _r; }
    inline void setG (unsigned char _g) { this->g = _g; }
    inline void setB (unsigned char _b) { this->b = _b; }
    inline void setA (double _a) { this->a = _a; }

    inline void apply (void) const { glColor4d(this->r, this->g, this->b, this->a); }

};

#endif
