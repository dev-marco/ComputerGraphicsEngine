#ifndef SRC_ENGINE_MESH_H_
#define SRC_ENGINE_MESH_H_

#include <valarray>
#include <vector>
#include <array>
#include <initializer_list>
#include <iostream>
#include <numeric>
#include <cmath>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "background.h"

namespace Engine {
    class Mesh {

        std::valarray<double> position, orientation;
        std::vector<Mesh *> children;

    public:

        static constexpr long double
            EPSILON = 0.0000000000000001L,
            PI = 3.141592653589793238462643383279502884L,
            DEG30 = PI / 6.0,
            DEG45 = PI / 4.0,
            DEG60 = PI / 3.0,
            DEG90 = PI / 2.0,
            DEG135 = DEG90 + DEG45,
            DEG180 = PI,
            DEG225 = -DEG135,
            DEG270 = -DEG90,
            DEG315 = -DEG45,
            DEG360 = PI + PI;

        static const std::valarray<double> quaternionIdentity, zero, axisX, axisY, axisZ;

        inline static bool equal (const std::valarray<double> &vec_1, const std::valarray<double> &vec_2) {
            if (vec_1.size() == vec_2.size()) {
                for (unsigned i = 0; i < vec_1.size(); ++i) {
                    if (vec_1[i] != vec_2[i]) {
                        return false;
                    }
                }
                return true;
            }
            return false;
        }

        inline static bool diff (const std::valarray<double> &vec_1, const std::valarray<double> &vec_2) {
            return !equal(vec_1, vec_2);
        }

        inline static std::valarray<double> lerp (const std::valarray<double> &vec_1, const std::valarray<double> &vec_2, const double position) {
            return (1.0 - position) * vec_1 + position * vec_2;
        }

        inline static double dot (const std::valarray<double> &vec_1, const std::valarray<double> &vec_2) {
            return (vec_1 * vec_2).sum();
        }

        template <typename T>
        inline static T clamp (const T value, const T min_value, const T max_value) {
            if (value > max_value) {
                return max_value;
            } else if (value < min_value) {
                return min_value;
            }
            return value;
        }

        inline static std::valarray<double> cross (const std::valarray<double> &vec_1, const std::valarray<double> &vec_2) {
            return {
                vec_1[1] * vec_2[2] - vec_2[1] * vec_1[2],
                vec_1[2] * vec_2[0] - vec_2[2] * vec_1[0],
                vec_1[0] * vec_2[1] - vec_2[0] * vec_1[1]
            };
        }

        inline static double norm2 (const std::valarray<double> &vec) {
            return dot(vec, vec);
        }

        inline static double norm (const std::valarray<double> &vec) {
            return std::sqrt(norm2(vec));
        }

        inline static std::valarray<double> resize (const std::valarray<double> &vec, const double vector_size, const double new_size) {
            return vec * (new_size / vector_size);
        }

        inline static std::valarray<double> resize (const std::valarray<double> &vec, const double new_size) {
            return resize(vec, norm(vec), new_size);
        }

        inline static std::valarray<double> unit (const std::valarray<double> &vec) {
            return resize(vec, 1.0);
        }

        inline static double distance (const std::valarray<double> &point_1, const std::valarray<double> &point_2) {
            return norm(point_1 - point_2);
        }

        inline static double distance2 (const std::valarray<double> &point_1, const std::valarray<double> &point_2) {
            return norm2(point_1 - point_2);
        }

        inline static std::valarray<double> axis2quat (const std::valarray<double> &axis, const double angle) {
            const double half_angle = angle * 0.5;
            const std::valarray<double> unit_vec = unit(axis) * std::sin(half_angle);
            return unit({
                unit_vec[0],
                unit_vec[1],
                unit_vec[2],
                std::cos(half_angle)
            });
        }

        inline static std::valarray<double> rot2quat (const std::valarray<double> &vec_1, const std::valarray<double> &vec_2) {
            static std::valarray<double> axis;
            constexpr double border = 1.0 - EPSILON;
            const double dot_prod = dot(vec_1, vec_2);
            if (dot_prod > border) {
                return quaternionIdentity;
            } else if (dot_prod < -border) {
                axis = cross(axisX, vec_1);
                if (norm2(axis) < EPSILON) {
                    axis = cross(axisY, vec_1);
                }
                return axis2quat(axis, DEG180);
            } else {
                axis = cross(vec_1, vec_2);
                return unit({ axis[0], axis[1], axis[2], 1 + dot_prod });
            }
        }

        inline static std::array<double, 16> quat2matrix (const std::valarray<double> &quat) {
            const double
                qi = quat[0], qj = quat[1], qk = quat[2], qr = quat[3],
                qii = qi * qi, qjj = qj * qj, qkk = qk * qk,
                qij = qi * qj, qik = qi * qk, qir = qi * qr,
                qjk = qj * qk, qjr = qj * qr,
                qkr = qk * qr,
                qiiii = qii + qii - 0.5, qjjjj = qjj + qjj - 0.5, qkkkk = qkk + qkk - 0.5,
                aijkr = qij + qkr, ajkir = qjk + qir, aikjr = qik + qjr,
                sijkr = qij - qkr, sjkir = qjk - qir, sikjr = qik - qjr;
            return {
                -(qjjjj + qkkkk),
                aijkr + aijkr,
                sikjr + sikjr,
                0.0,

                sijkr + sijkr,
                -(qiiii + qkkkk),
                ajkir + ajkir,
                0.0,

                aikjr + aikjr,
                sjkir + sjkir,
                -(qiiii + qjjjj),
                0.0,

                0.0, 0.0, 0.0, 1.0
            };
        }

        inline static std::valarray<double> rotate (const std::valarray<double> &vec, const std::valarray<double> &quat) {
            const std::array<double, 16> matrix = quat2matrix(quat);
            return {
                vec[0] * matrix[0] + vec[1] * matrix[4] + vec[2] * matrix[ 8],
                vec[0] * matrix[1] + vec[1] * matrix[5] + vec[2] * matrix[ 9],
                vec[0] * matrix[2] + vec[1] * matrix[6] + vec[2] * matrix[10]
            };
        }

        inline static std::valarray<double> rotate (const std::valarray<double> &point, const std::valarray<double> &quat, const std::valarray<double> &pivot) {
            return quat * (point - pivot) + pivot;
        }

        inline static double areaTriangle (
            const std::valarray<double> &tri_point_1,
            const std::valarray<double> &tri_point_2,
            const std::valarray<double> &tri_point_3
        ) {
            if (tri_point_1[2] == tri_point_2[2] && tri_point_2[2] == tri_point_3[2]) {
                return std::abs((
                    tri_point_1[0] * (tri_point_2[1] - tri_point_3[1]) +
                    tri_point_2[0] * (tri_point_3[1] - tri_point_1[1]) +
                    tri_point_3[0] * (tri_point_1[1] - tri_point_2[1])
                ) * 0.5);
            } else {
                const double
                    a2 = norm2(tri_point_2 - tri_point_1),
                    b2 = norm2(tri_point_3 - tri_point_2),
                    c2 = norm2(tri_point_1 - tri_point_3),
                    a2b2c2 = a2 + b2 - c2;
                return std::sqrt(4.0 * a2 * b2 + a2b2c2 * a2b2c2) * 0.25;
            }
        }

        inline static std::array<std::array<std::valarray<double>, 2>, 3> edgesTriangle2D (
            const std::valarray<double> &tri_point_1,
            const std::valarray<double> &tri_point_2,
            const std::valarray<double> &tri_point_3
        ) {
            return {{
                { tri_point_1, tri_point_2 },
                { tri_point_2, tri_point_3 },
                { tri_point_3, tri_point_1 }
            }};
        }

        inline static double areaRectangle (
            const std::valarray<double> &rect_top_left,
            const std::valarray<double> &rect_bottom_left,
            const std::valarray<double> &rect_bottom_right
        ) {
            return std::sqrt(norm2(rect_bottom_left - rect_top_left) * norm2(rect_bottom_right - rect_bottom_left));
        }

        inline static std::array<std::array<std::valarray<double>, 2>, 4> edgesRectangle2D (
            const std::valarray<double> &rect_top_left,
            const std::valarray<double> &rect_bottom_left,
            const std::valarray<double> &rect_bottom_right,
            const std::valarray<double> &rect_top_right
        ) {
            return {{
                { rect_top_left, rect_bottom_left },
                { rect_bottom_left, rect_bottom_right },
                { rect_bottom_right, rect_top_right },
                { rect_top_right, rect_top_left }
            }};
        }

        static double distanceRayToPoint (
            const std::valarray<double> &ray_start,
            const std::valarray<double> &ray_end,
            const std::valarray<double> &point,
            std::valarray<double> &near_point,
            bool infinite = false
        ) {
            const std::valarray<double> delta_ray = ray_end - ray_start;
            const double length2 = norm2(delta_ray);

            if (length2 == 0.0) {
                near_point = ray_start;
            } else {
                double param = ((point - ray_start) * delta_ray).sum() / length2;

                if (!infinite) {
                    param = clamp(param, 0.0, 1.0);
                }

                near_point = ray_start + param * delta_ray;
            }

            return distance(point, near_point);
        }

        template <typename T>
        static bool collisionPointConvexPolygon2D (
            const std::valarray<double> &point,
            T edges_ccw
        ) {
            for (const std::array<std::valarray<double>, 2> &edge : edges_ccw) {
                const double
                    A = edge[0][1] - edge[1][1],
                    B = edge[1][0] - edge[0][0];
                if ((A * point[0] + B * point[1]) > (A * edge[0][0] + B * edge[0][1])) {
                    return false;
                }
            }

            return true;
        }

        inline static bool collisionPointTriangle2D (
            const std::valarray<double> &point,
            const std::valarray<double> &tri_point_1,
            const std::valarray<double> &tri_point_2,
            const std::valarray<double> &tri_point_3
        ) {
            return collisionPointConvexPolygon2D(point, edgesTriangle2D(tri_point_1, tri_point_2, tri_point_3));
        };

        inline static bool collisionPointRectangle2D (
            const std::valarray<double> &point,
            const std::valarray<double> &rect_top_left,
            const std::valarray<double> &rect_bottom_left,
            const std::valarray<double> &rect_bottom_right,
            const std::valarray<double> &rect_top_right
        ) {
            return collisionPointConvexPolygon2D(point, edgesRectangle2D(rect_top_left, rect_bottom_left, rect_bottom_right, rect_top_right));
        }

        inline static bool collisionSpheres (
            const std::valarray<double> &position_1,
            const double radius_1,
            const std::valarray<double> &position_2,
            const double radius_2
        ) {
            const double center_distance = radius_1 + radius_2;
            return distance2(position_1, position_2) <= (center_distance * center_distance);
        }

        static double distanceRays (
            const std::valarray<double> &ray_1_start,
            const std::valarray<double> &ray_1_end,
            const std::valarray<double> &ray_2_start,
            const std::valarray<double> &ray_2_end,
            std::valarray<double> &ray_start,
            std::valarray<double> &ray_end
        );

        static bool collisionRectangles2D (
            const std::valarray<double> &rect_1_top_left,
            const std::valarray<double> &rect_1_bottom_left,
            const std::valarray<double> &rect_1_bottom_right,
            const std::valarray<double> &rect_1_top_right,
            const std::valarray<double> &rect_1_orientation,
            const std::valarray<double> &rect_2_top_left,
            const std::valarray<double> &rect_2_bottom_left,
            const std::valarray<double> &rect_2_bottom_right,
            const std::valarray<double> &rect_2_top_right,
            const std::valarray<double> &rect_2_orientation,
            std::valarray<double> &near_point
        );

        inline static bool collisionRaySphere (
            const std::valarray<double> &ray_start,
            const std::valarray<double> &ray_end,
            const std::valarray<double> &circle_center,
            const double circle_radius,
            std::valarray<double> &near_point,
            const bool infinite = false
        ) { return distanceRayToPoint(ray_start, ray_end, circle_center, near_point, infinite) <= circle_radius; }

        inline static bool collisionRectangleCircle2D (
            const std::valarray<double> &rect_top_left,
            const std::valarray<double> &rect_bottom_left,
            const std::valarray<double> &rect_bottom_right,
            const std::valarray<double> &rect_top_right,
            const std::valarray<double> &circle_center,
            const double circle_radius,
            std::valarray<double> &near_point
        ) {

            if (collisionRaySphere(rect_top_left, rect_top_right, circle_center, circle_radius, near_point) ||
                collisionRaySphere(rect_top_right, rect_bottom_right, circle_center, circle_radius, near_point) ||
                collisionRaySphere(rect_bottom_left, rect_bottom_right, circle_center, circle_radius, near_point) ||
                collisionRaySphere(rect_top_left, rect_bottom_left, circle_center, circle_radius, near_point)) {
                return true;
            }

            if (collisionPointRectangle2D(rect_top_left, rect_bottom_left, rect_bottom_right, rect_top_right, circle_center)) {
                near_point = circle_center;
                return true;
            }

            return false;
        }

        Mesh (const std::valarray<double> &_position = zero, const std::valarray<double> _orientation = quaternionIdentity) :
            position(_position), orientation(_orientation) {};

        virtual ~Mesh () {}

        void draw (const Background *background, const bool only_border = false) const {

            const std::valarray<double>
                translation = this->getPosition(),
                orientation = this->getOrientation();

            glPushMatrix();

            if (diff(translation, zero)) {
                glTranslated(translation[0], translation[1], translation[2]);
            }

            if (diff(orientation, quaternionIdentity)) {
                glMultMatrixd(quat2matrix(orientation).data());
            }

            this->_draw(background, only_border);

            if (!this->children.empty()) {
                for (const auto &mesh : this->children) {
                    mesh->draw(background, only_border);
                }
            }

            glPopMatrix();
        }

        inline void addChild (Mesh *child) { this->children.push_back(child); }

        inline virtual void _draw (const Background *background, const bool only_border) const {}

        inline virtual Mesh *getCollisionSpace (const std::valarray<double> &speed) const { return nullptr; }

        inline virtual bool _detectCollision (
            const Mesh *other,
            const std::valarray<double> &my_offset,
            const std::valarray<double> &other_offset,
            std::valarray<double> &point,
            const bool try_inverse = true
        ) const { return false; }

        inline bool detectCollision (
            Mesh *other,
            const std::valarray<double> &my_offset,
            const std::valarray<double> &my_speed,
            const std::valarray<double> &other_offset,
            const std::valarray<double> &other_speed,
            std::valarray<double> &point,
            const bool try_inverse = true
        ) {

            if (this->_detectCollision(other, my_offset, other_offset, point, try_inverse)) {
                return true;
            }

            if (diff(my_speed, zero)) {
                const std::valarray<double> stopped = zero;
                const std::unique_ptr<const Mesh> my_space(this->getCollisionSpace(my_speed));

                if (my_space) {

                    if (my_space->_detectCollision(other, my_offset, other_offset, point, try_inverse)) {
                        std::cout << "first case " << this->getType() << " " << other->getType() << std::endl;
                        return true;
                    }

                    if (try_inverse && diff(other_speed, zero)) {

                        const std::unique_ptr<const Mesh> other_space(other->getCollisionSpace(other_speed));

                        if (
                            other_space &&
                            my_space->_detectCollision(other_space.get(), my_offset, other_offset, point, try_inverse)
                        ) {
                            std::cout << "second case " << this->getType() << " " << other->getType() << std::endl;
                            return true;
                        }
                    }
                }
            }

            if (try_inverse) {
                return other->detectCollision(this, other_offset, other_speed, my_offset, my_speed, point, false);
            }
            return false;
        }

        inline const std::valarray<double> &getOrientation (void) const { return this->orientation; }
        inline void setOrientation (const std::valarray<double> &_orientation) { this->orientation = _orientation; }

        inline const std::valarray<double> &getPosition () const { return this->position; }
        inline void setPosition (const std::valarray<double> &_position) { this->position = _position; }

        inline virtual const std::string getType (void) const { return "mesh"; }
    };

    class Rectangle2D : public Mesh {

        double width, height;
        std::valarray<double> top_left, top_right, bottom_left, bottom_right;

    public:
        inline Rectangle2D (const std::valarray<double> &_position, double _width, double _height, const std::valarray<double> _orientation = quaternionIdentity) :
            Mesh(_position, _orientation), width(_width), height(_height) {
            this->updatePositions();
        }

        void updatePositions (void) {
            const std::valarray<double>
                top_left = this->getPosition(),
                bottom_right = { top_left[0] + this->getWidth(), top_left[1] - this->getHeight(), top_left[2] };

            this->top_left = rotate(top_left, this->getOrientation());
            this->bottom_left = rotate({ top_left[0], bottom_right[1], top_left[2] }, this->getOrientation());
            this->bottom_right = rotate(bottom_right, this->getOrientation());
            this->top_right = rotate({ bottom_right[0], top_left[1], top_left[2] }, this->getOrientation());
        }

        inline double getWidth (void) const { return this->width; }
        inline double getHeight (void) const { return this->height; }

        inline void setWidth (const double _width) { this->width = _width, this->updatePositions(); }
        inline void setHeight (const double _height) { this->height = _height, this->updatePositions(); }
        inline void setOrientation (const std::valarray<double> &_orientation) { Mesh::setOrientation(_orientation), this->updatePositions(); }
        inline void setPosition (const std::valarray<double> &_position) { Mesh::setPosition(_position), this->updatePositions(); }

        inline const std::valarray<double> &getTopLeftPosition (void) const { return this->top_left; }
        inline const std::valarray<double> &getTopRightPosition (void) const { return this->top_right; }
        inline const std::valarray<double> &getBottomLeftPosition (void) const { return this->bottom_left; }
        inline const std::valarray<double> &getBottomRightPosition (void) const { return this->bottom_right; }

        void _draw (const Background *background, const bool only_border) const {

            const std::valarray<double>
                top_left = this->getPosition(),
                bottom_right = { top_left[0] + this->getWidth(), top_left[1] - this->getHeight(), top_left[2] };

            if (only_border) {
                glBegin(GL_LINE_LOOP);
            } else {
                glBegin(GL_TRIANGLE_FAN);
            }

            background->apply();

            glVertex3d(top_left[0], top_left[1], top_left[2]);
            glVertex3d(top_left[0], bottom_right[1], top_left[2]);
            glVertex3d(bottom_right[0], bottom_right[1], top_left[2]);
            glVertex3d(bottom_right[0], top_left[1], top_left[2]);

            glEnd();
        }

        inline bool _detectCollision (
            const Mesh *other,
            const std::valarray<double> &my_offset,
            const std::valarray<double> &other_offset,
            std::valarray<double> &point,
            const bool try_inverse = true
        ) const {

            if (other->getType() == "rectangle2d") {
                const Rectangle2D *rect = static_cast<const Rectangle2D *>(other);
                return Mesh::collisionRectangles2D(
                    my_offset + this->getTopLeftPosition(),
                    my_offset + this->getBottomLeftPosition(),
                    my_offset + this->getBottomRightPosition(),
                    my_offset + this->getTopRightPosition(),
                    this->getOrientation(),
                    other_offset + rect->getTopLeftPosition(),
                    other_offset + rect->getBottomLeftPosition(),
                    other_offset + rect->getBottomRightPosition(),
                    other_offset + rect->getTopRightPosition(),
                    other->getOrientation(),
                    point
                );
            }

            return false;
        }

        inline const std::string getType (void) const { return "rectangle2d"; }

    };

    class Polygon2D : public Mesh {

        double radius;
        int sides;

    public:

        inline Polygon2D (const std::valarray<double> &_position, double _radius, int _sides, const std::valarray<double> _orientation = quaternionIdentity) :
            Mesh(_position, _orientation), radius(_radius), sides(_sides) {};

        void _draw (const Background *background, const bool only_border) const {

            const std::valarray<double> position = this->getPosition();
            const double step = (Polygon2D::PI * 2.0) / static_cast<double>(this->sides);

            double ang = 0.0;

            if (only_border) {
                glBegin(GL_LINE_LOOP);
            } else {
                glBegin(GL_TRIANGLE_FAN);
            }

            background->apply();

            for (int i = 0; i < this->sides; i++) {

                glVertex3d(position[0] + this->getRadius() * std::cos(ang), position[1] + this->getRadius() * std::sin(ang), position[2]);
                ang += step;

            }

            glEnd();
        }

        inline double getRadius (void) const { return this->radius; }
        inline void setRadius (const double _radius) { this->radius = _radius; }

        Mesh *getCollisionSpace (const std::valarray<double> &speed) const {

            if (norm2(speed) > (this->getRadius() * this->getRadius())) {
                const double speed_angle = std::atan2(speed[1], speed[0]);

                const std::valarray<double>
                    difference = {
                        this->getRadius() * static_cast<double>(std::cos(speed_angle + DEG90)),
                        this->getRadius() * static_cast<double>(std::sin(speed_angle + DEG90)),
                        0.0
                    },
                    top_position = this->getPosition() + difference;

                return new Rectangle2D(top_position, Mesh::norm(speed), this->getRadius() * 2.0, rot2quat(unit(speed), axisX));
            } else {
                return nullptr;
            }
        }

        bool _detectCollision (
            const Mesh *other,
            const std::valarray<double> &my_offset,
            const std::valarray<double> &other_offset,
            std::valarray<double> &point,
            const bool try_inverse = true
        ) const {

            if (other->getType() == "polygon2d" || other->getType() == "sphere2d") {
                const Polygon2D *poly = static_cast<const Polygon2D *>(other);
                if (Mesh::collisionSpheres(my_offset + this->getPosition(), this->getRadius(), other_offset + other->getPosition(), poly->getRadius())) {
                    point = (this->getPosition() + other->getPosition()) * 0.5;
                    return true;
                }
            } else if (other->getType() == "rectangle2d") {
                const Rectangle2D *rect = static_cast<const Rectangle2D *>(other);
                return Mesh::collisionRectangleCircle2D(
                    other_offset + rect->getTopLeftPosition(),
                    other_offset + rect->getBottomLeftPosition(),
                    other_offset + rect->getBottomRightPosition(),
                    other_offset + rect->getTopRightPosition(),
                    my_offset + this->getPosition(),
                    this->getRadius(),
                    point
                );
            }

            return false;
        }

        inline const std::string getType (void) const { return "polygon2d"; }

    };

    class Sphere2D : public Polygon2D {
    public:
        Sphere2D (const std::valarray<double> &_position, const double _radius) :
            Polygon2D (_position, _radius, _radius * 20, quaternionIdentity) {}

        inline const std::string getType (void) const { return "sphere2d"; }
    };
};

#endif
