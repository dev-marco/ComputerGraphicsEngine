#ifndef SRC_ENGINE_QUATERNION_H_
#define SRC_ENGINE_QUATERNION_H_

#include "defaults.h"
#include "vec.h"

namespace Engine {
    class Quaternion : public Vec<4> {

    public:

        static const Quaternion identity;

        static Quaternion axisAngle (const Vec<3> &axis, float_max_t angle) {
            const float_max_t half_angle = angle * 0.5;
            const Vec<3> unit_vec = axis.normalized() * std::sin(half_angle);

            return Quaternion(
                unit_vec[0],
                unit_vec[1],
                unit_vec[2],
                std::cos(half_angle)
            );
        }

// -------------------------------------

        static Quaternion difference (const Vec<3> &vec_1, const Vec<3> &vec_2) {
            static Vec<3> axis;
            constexpr float_max_t border = 1.0 - EPSILON;
            const float_max_t dot_prod = vec_1.dot(vec_2);
            if (dot_prod > border) {
                return identity;
            } else if (dot_prod < -border) {
                axis = Vec<3>::axisX.cross(vec_1);
                if (axis.length2() < EPSILON) {
                    axis = Vec<3>::axisY.cross(vec_1);
                }
                return axisAngle(axis, DEG180);
            }
            axis = vec_1.cross(vec_2);
            return Quaternion(axis[0], axis[1], axis[2], 1.0 + dot_prod);
        }

// -----------------------------------------------------------------------------

        template <
            typename TYPE_C,
            typename = typename std::enable_if<std::is_convertible<TYPE_C, Vec<4>>::value, TYPE_C>::type
        >
        inline Quaternion (const TYPE_C &vec) : Vec<4>(vec) {}

        inline Quaternion (float_max_t x, float_max_t y, float_max_t z, float_max_t w) : Vec<4>({ x, y, z, w }) {
            this->normalize();
        }

// -----------------------------------------------------------------------------

        inline std::array<float_max_t, 16> rotation (void) const {
            const float_max_t
                qi = this->store[0], qj = this->store[1],
                qk = this->store[2], qr = this->store[3],
                qii = qi * qi, qjj = qj * qj, qkk = qk * qk,
                qij = qi * qj, qik = qi * qk, qir = qi * qr,
                qjk = qj * qk, qjr = qj * qr,
                qkr = qk * qr,
                qiiii = (qii + qii) - 0.5, qjjjj = (qjj + qjj) - 0.5,
                qkkkk = (qkk + qkk) - 0.5,
                aijkr = qij + qkr, ajkir = qjk + qir, aikjr = qik + qjr,
                sijkr = qij - qkr, sjkir = qjk - qir, sikjr = qik - qjr;

            return {
                -(qjjjj + qkkkk), (aijkr + aijkr), (sikjr + sikjr), 0.0,

                (sijkr + sijkr), -(qiiii + qkkkk), (ajkir + ajkir), 0.0,

                (aikjr + aikjr), (sjkir + sjkir), -(qiiii + qjjjj), 0.0,

                0.0, 0.0, 0.0, 1.0
            };
        }

// -------------------------------------

        inline Vec<3> rotated (const Vec<3> &vec, const Vec<3> &pivot = Vec<3>::zero) const {
            if (!this->isIdentity()) {
                const std::array<float_max_t, 16> matrix = this->rotation();
                const std::array<float_max_t, 3> diff = {
                    vec[0] - pivot[0], vec[1] - pivot[1], vec[2] - pivot[2]
                };
                return {
                    pivot[0] + (diff[0] * matrix[0]) + (diff[1] * matrix[4]) + (diff[1] * matrix[ 8]),
                    pivot[1] + (diff[0] * matrix[1]) + (diff[1] * matrix[5]) + (diff[1] * matrix[ 9]),
                    pivot[2] + (diff[0] * matrix[2]) + (diff[1] * matrix[6]) + (diff[1] * matrix[10])
                };
            }
            return vec;
        }

        inline Vec<3> &rotate (Vec<3> &vec, const Vec<3> &pivot = Vec<3>::zero) const {
            if (!this->isIdentity()) {
                const std::array<float_max_t, 16> matrix = this->rotation();
                const std::array<float_max_t, 3> diff = {
                    vec[0] - pivot[0], vec[1] - pivot[1], vec[2] - pivot[2]
                };
                vec[0] = pivot[0] + (diff[0] * matrix[0]) + (diff[1] * matrix[4]) + (diff[1] * matrix[ 8]);
                vec[1] = pivot[1] + (diff[0] * matrix[1]) + (diff[1] * matrix[5]) + (diff[1] * matrix[ 9]);
                vec[2] = pivot[2] + (diff[0] * matrix[2]) + (diff[1] * matrix[6]) + (diff[1] * matrix[10]);
            }
            return vec;
        }

// -----------------------------------------------------------------------------

        inline bool isIdentity (void) const { return (*this) == identity; }
    };
};

#endif