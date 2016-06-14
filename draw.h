#ifndef SRC_ENGINE_DRAW_H_
#define SRC_ENGINE_DRAW_H_

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <xmmintrin.h>
#include "defaults.h"
#include "vec.h"
#include "quaternion.h"
#include "background.h"
#include "shader.h"

namespace Engine {

    class Draw {

    static unsigned drawn;

    static std::stack<std::array<float_max_t, 16>> matrix_stack;
    static std::array<float_max_t, 16> matrix;
    static Background *background;

    static void multiplyMatrix (const std::array<float_max_t, 16> &to) {

        // std::array<float_max_t, 16> result;
        // const float_max_t *A = &to[0], *B = &matrix[0];
        // float_max_t *C = &result[0];
        // __m256d
        //     col1 = _mm256_load_pd(&B[ 0]),
        //     col2 = _mm256_load_pd(&B[ 4]),
        //     col3 = _mm256_load_pd(&B[ 8]),
        //     col4 = _mm256_load_pd(&B[12]);
        // for(unsigned i = 0; i < 4; ++i) {
        //     unsigned row = i << 2;
        //     _mm256_store_pd(&C[row], _mm256_add_pd(
        //             _mm256_add_pd(
        //                 _mm256_mul_pd(_mm256_set1_pd(A[row]), col1),
        //                 _mm256_mul_pd(_mm256_set1_pd(A[row + 1]), col2)
        //             ),
        //             _mm256_add_pd(
        //                 _mm256_mul_pd(_mm256_set1_pd(A[row + 2]), col3),
        //                 _mm256_mul_pd(_mm256_set1_pd(A[row + 3]), col4)
        //             )
        //         )
        //     );
        // }
        // matrix.swap(result);

        // matrix = {
        //     matrix[ 0] * to[ 0] + matrix[ 1] * to[ 4] + matrix[ 2] * to[ 8] + matrix[ 3] * to[12],
        //     matrix[ 0] * to[ 1] + matrix[ 1] * to[ 5] + matrix[ 2] * to[ 9] + matrix[ 3] * to[13],
        //     matrix[ 0] * to[ 2] + matrix[ 1] * to[ 6] + matrix[ 2] * to[10] + matrix[ 3] * to[14],
        //     matrix[ 0] * to[ 3] + matrix[ 1] * to[ 7] + matrix[ 2] * to[11] + matrix[ 3] * to[15],
        //
        //     matrix[ 4] * to[ 0] + matrix[ 5] * to[ 4] + matrix[ 6] * to[ 8] + matrix[ 7] * to[12],
        //     matrix[ 4] * to[ 1] + matrix[ 5] * to[ 5] + matrix[ 6] * to[ 9] + matrix[ 7] * to[13],
        //     matrix[ 4] * to[ 2] + matrix[ 5] * to[ 6] + matrix[ 6] * to[10] + matrix[ 7] * to[14],
        //     matrix[ 4] * to[ 3] + matrix[ 5] * to[ 7] + matrix[ 6] * to[11] + matrix[ 7] * to[15],
        //
        //     matrix[ 8] * to[ 0] + matrix[ 9] * to[ 4] + matrix[10] * to[ 8] + matrix[11] * to[12],
        //     matrix[ 8] * to[ 1] + matrix[ 9] * to[ 5] + matrix[10] * to[ 9] + matrix[11] * to[13],
        //     matrix[ 8] * to[ 2] + matrix[ 9] * to[ 6] + matrix[10] * to[10] + matrix[11] * to[14],
        //     matrix[ 8] * to[ 3] + matrix[ 9] * to[ 7] + matrix[10] * to[11] + matrix[11] * to[15],
        //
        //     matrix[12] * to[ 0] + matrix[13] * to[ 4] + matrix[14] * to[ 8] + matrix[15] * to[12],
        //     matrix[12] * to[ 1] + matrix[13] * to[ 5] + matrix[14] * to[ 9] + matrix[15] * to[13],
        //     matrix[12] * to[ 2] + matrix[13] * to[ 6] + matrix[14] * to[10] + matrix[15] * to[14],
        //     matrix[12] * to[ 3] + matrix[13] * to[ 7] + matrix[14] * to[11] + matrix[15] * to[15]
        // };
    }

    static inline void transformVertex (Vec<3> &vertex) {
        // vertex[0] = (vertex[0] * matrix[0]) + (vertex[1] * matrix[4]) + (vertex[1] * matrix[ 8]) + matrix[12];
        // vertex[1] = (vertex[0] * matrix[1]) + (vertex[1] * matrix[5]) + (vertex[1] * matrix[ 9]) + matrix[13];
        // vertex[2] = (vertex[0] * matrix[2]) + (vertex[1] * matrix[6]) + (vertex[1] * matrix[10]) + matrix[14];
    }

    public:

        // TODO create camera
        inline static void lookAt (const Engine::Vec<3> &eye_pos, const Engine::Vec<3> &look_dir, const Engine::Vec<3> &up_vec) {

            glMatrixMode(GL_MODELVIEW);
            glLoadIdentity();

            gluLookAt(
                eye_pos[0], eye_pos[1], eye_pos[2],
                look_dir[0], look_dir[1], look_dir[2],
                up_vec[0], up_vec[1], up_vec[2]
            );
        }

        inline static void perspective (float_max_t fovy, float_max_t aspect, float_max_t zNear = 1.0, float_max_t zFar = 100.0) {
            glMatrixMode(GL_PROJECTION);
            glLoadIdentity();
            gluPerspective(fovy, aspect, zNear, zFar);
            glMatrixMode(GL_MODELVIEW);
        }

// -----------------------------------------------------------------------------

        inline static void begin (Background *_background = nullptr) {
            // TODO lock
            background = _background;
            glBegin(GL_TRIANGLES);
            drawn = 0;
        }

        inline static unsigned end (void) {
            unsigned total = drawn;
            glEnd();
            background = nullptr;
            // TODO unlock
            return total;
        }

// -----------------------------------------------------------------------------

        inline static void vertex (Vec<3> vert) {
            if (background) {
                background->apply();
            }
            ++drawn;
            // transformVertex(vert);
            glVertex3dv(vert.data());
        }

        inline static void vertex (float_max_t vert_0, float_max_t vert_1, float_max_t vert_2) {
            vertex({ vert_0, vert_1, vert_2 });
        }

// -------------------------------------

        inline static void normal (Vec<3> vec) {
            glNormal3dv(vec.data());
        }

        inline static void normal (float_max_t vert_0, float_max_t vert_1, float_max_t vert_2) {
            normal({ vert_0, vert_1, vert_2 });
        }

// -----------------------------------------------------------------------------

        inline static void translate (float_max_t vert_0, float_max_t vert_1, float_max_t vert_2) {
            if (vert_0 || vert_1 || vert_2) {
                multiplyMatrix({
                       1.0,    0.0,   0.0,  0.0,
                       0.0,    1.0,   0.0,  0.0,
                       0.0,    0.0,   1.0,  0.0,
                    vert_0, vert_1, vert_2, 1.0
                });
                glTranslated(vert_0, vert_1, vert_2);
            }
        }

        inline static void translate (const Vec<3> &vec) {
            translate(vec[0], vec[1], vec[2]);
        }

// -------------------------------------

        inline static void rotate (const Quaternion &quat) {
            if (!quat.isIdentity()) {
                multiplyMatrix(quat.rotation());
                glMultMatrixd(quat.rotation().data());
            }
        }

// -----------------------------------------------------------------------------

        inline static void push (void) {
            matrix_stack.push(matrix);
            glPushMatrix();
        }

        inline static void pop (void) {
            matrix = matrix_stack.top();
            matrix_stack.pop();
            glPopMatrix();
        }

    };

};

#endif
