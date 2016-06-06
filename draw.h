#ifndef SRC_ENGINE_DRAW_H_
#define SRC_ENGINE_DRAW_H_

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "defaults.h"
#include "vec.h"
#include "quaternion.h"

namespace Engine {

    class Draw {

    static unsigned drawn;

    public:

        inline static void begin (GLuint type) {
            drawn = 0;
            glBegin(type);
        }

        inline static unsigned end (void) {
            glEnd();
            return drawn;
        }

// -----------------------------------------------------------------------------

        inline static void vertex (const Vec<3> &vert) {
            ++drawn;
            glVertex3d(vert[0], vert[1], vert[2]);
        }

        inline static void vertex (float_max_t vert_0, float_max_t vert_1, float_max_t vert_2) {
            ++drawn;
            glVertex3d(vert_0, vert_1, vert_2);
        }

// -------------------------------------

        inline static void normal (const Vec<3> &vec) {
            glNormal3d(vec[0], vec[1], vec[2]);
        }

        inline static void normal (float_max_t vert_0, float_max_t vert_1, float_max_t vert_2) {
            glNormal3d(vert_0, vert_1, vert_2);
        }

// -----------------------------------------------------------------------------

        inline static void translate (const Vec<3> &vec) {
            if (vec) {
                glTranslated(vec[0], vec[1], vec[2]);
            }
        }

        inline static void translate (float_max_t vert_0, float_max_t vert_1, float_max_t vert_2) {
            if (vert_0 || vert_1 || vert_2) {
                glTranslated(vert_0, vert_1, vert_2);
            }
        }

// -------------------------------------

        inline static void rotate (const Quaternion &quat) {
            if (!quat.isIdentity()) {
                glMultMatrixd(quat.rotation().data());
            }
        }

// -----------------------------------------------------------------------------

        inline static void push (void) {
            glPushMatrix();
        }

        inline static void pop (void) {
            glPopMatrix();
        }

    };

};

#endif
