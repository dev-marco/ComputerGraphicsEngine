#ifndef SRC_ENGINE_MESH_H_
#define SRC_ENGINE_MESH_H_

#include <vector>
#include <array>
#include <initializer_list>
#include <iostream>
#include <numeric>
#include <cmath>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "defaults.h"
#include "vec.h"
#include "quaternion.h"
#include "draw.h"
#include "background.h"

namespace Engine {
    class Mesh {

        Vec<3> position;
        Quaternion orientation;
        std::vector<Mesh *> children;

    public:

        template <typename T>
        inline static constexpr T clamp (const T value, const T min_value, const T max_value) {
            if (value > max_value) {
                return max_value;
            } else if (value < min_value) {
                return min_value;
            }
            return value;
        }

        inline static float_max_t areaTriangle (
            const Vec<3> &tri_point_1,
            const Vec<3> &tri_point_2,
            const Vec<3> &tri_point_3
        ) {
            if (tri_point_1[2] == tri_point_2[2] && tri_point_2[2] == tri_point_3[2]) {
                return std::abs((
                    tri_point_1[0] * (tri_point_2[1] - tri_point_3[1]) +
                    tri_point_2[0] * (tri_point_3[1] - tri_point_1[1]) +
                    tri_point_3[0] * (tri_point_1[1] - tri_point_2[1])
                ) * 0.5);
            } else {
                const float_max_t
                    a2 = (tri_point_2 - tri_point_1).length2(),
                    b2 = (tri_point_3 - tri_point_2).length2(),
                    c2 = (tri_point_1 - tri_point_3).length2(),
                    a2b2c2 = a2 + b2 - c2;
                return std::sqrt(4.0 * a2 * b2 + a2b2c2 * a2b2c2) * 0.25;
            }
        }

        inline static constexpr std::array<std::array<Vec<3>, 2>, 3> edgesTriangle (
            const Vec<3> &tri_point_1,
            const Vec<3> &tri_point_2,
            const Vec<3> &tri_point_3
        ) {
            return {{
                { tri_point_1, tri_point_2 },
                { tri_point_2, tri_point_3 },
                { tri_point_3, tri_point_1 }
            }};
        }

        inline static float_max_t areaRectangle (
            const Vec<3> &rect_top_left,
            const Vec<3> &rect_bottom_left,
            const Vec<3> &rect_bottom_right
        ) {
            return std::sqrt((rect_bottom_left - rect_top_left).length2() * (rect_bottom_right - rect_bottom_left).length2());
        }

        inline static constexpr std::array<std::array<Vec<3>, 2>, 4> edgesRectangle (
            const Vec<3> &rect_top_left,
            const Vec<3> &rect_bottom_left,
            const Vec<3> &rect_bottom_right,
            const Vec<3> &rect_top_right
        ) {
            return {{
                { rect_top_left, rect_bottom_left },
                { rect_bottom_left, rect_bottom_right },
                { rect_bottom_right, rect_top_right },
                { rect_top_right, rect_top_left }
            }};
        }

        static float_max_t distanceRayToPoint (
            const Vec<3> &ray_start,
            const Vec<3> &ray_end,
            const Vec<3> &point,
            Vec<3> &near_point,
            bool infinite = false
        ) {
            const Vec<3> delta_ray = ray_end - ray_start;
            const float_max_t length_2 = delta_ray.length2();

            if (length_2 == 0.0) {
                near_point = ray_start;
            } else {
                float_max_t param = ((point - ray_start) * delta_ray).sum() / length_2;

                if (!infinite) {
                    param = clamp(param, 0.0, 1.0);
                }

                near_point = ray_start + param * delta_ray;
            }

            return point.distance(near_point);
        }

        template <typename T>
        static constexpr bool collisionPointConvexPolygon2D (
            const Vec<3> &point,
            T edges_ccw
        ) {
            for (const std::array<Vec<3>, 2> &edge : edges_ccw) {
                const float_max_t
                    A = edge[0][1] - edge[1][1],
                    B = edge[1][0] - edge[0][0];
                if ((A * point[0] + B * point[1]) > (A * edge[0][0] + B * edge[0][1])) {
                    return false;
                }
            }

            return true;
        }

        inline static constexpr bool collisionPointTriangle2D (
            const Vec<3> &point,
            const Vec<3> &tri_point_1,
            const Vec<3> &tri_point_2,
            const Vec<3> &tri_point_3
        ) {
            return collisionPointConvexPolygon2D(point, edgesTriangle(tri_point_1, tri_point_2, tri_point_3));
        };

        inline static constexpr bool collisionPointRectangle2D (
            const Vec<3> &point,
            const Vec<3> &rect_top_left,
            const Vec<3> &rect_bottom_left,
            const Vec<3> &rect_bottom_right,
            const Vec<3> &rect_top_right
        ) {
            return collisionPointConvexPolygon2D(point, edgesRectangle(rect_top_left, rect_bottom_left, rect_bottom_right, rect_top_right));
        }

        inline static constexpr bool collisionSpheres (
            const Vec<3> &position_1,
            const float_max_t radius_1,
            const Vec<3> &position_2,
            const float_max_t radius_2
        ) {
            const float_max_t center_distance = radius_1 + radius_2;
            return position_1.distance2(position_2) <= (center_distance * center_distance);
        }

        static float_max_t distanceRays (
            const Vec<3> &ray_1_start,
            const Vec<3> &ray_1_end,
            const Vec<3> &ray_2_start,
            const Vec<3> &ray_2_end,
            Vec<3> &ray_start,
            Vec<3> &ray_end
        );

        static bool collisionRectangles (
            const Vec<3> &rect_1_top_left,
            const Vec<3> &rect_1_bottom_left,
            const Vec<3> &rect_1_bottom_right,
            const Vec<3> &rect_1_top_right,
            const Quaternion &rect_1_orientation,
            const Vec<3> &rect_2_top_left,
            const Vec<3> &rect_2_bottom_left,
            const Vec<3> &rect_2_bottom_right,
            const Vec<3> &rect_2_top_right,
            const Quaternion &rect_2_orientation,
            Vec<3> &near_point
        );

        inline static bool collisionRaySphere (
            const Vec<3> &ray_start,
            const Vec<3> &ray_end,
            const Vec<3> &circle_center,
            const float_max_t circle_radius,
            Vec<3> &near_point,
            const bool infinite = false
        ) { return distanceRayToPoint(ray_start, ray_end, circle_center, near_point, infinite) <= circle_radius; }

        inline static bool collisionRectangleCircle2D (
            const Vec<3> &rect_top_left,
            const Vec<3> &rect_bottom_left,
            const Vec<3> &rect_bottom_right,
            const Vec<3> &rect_top_right,
            const Vec<3> &circle_center,
            const float_max_t circle_radius,
            Vec<3> &near_point
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

        Mesh (const Vec<3> &_position = Vec<3>::zero, const Quaternion &_orientation = Quaternion::identity) :
            position(_position), orientation(_orientation) {};

        virtual ~Mesh () {}

        void draw(const Background *background, const bool only_border = false) const;

        inline void addChild (Mesh *child) { this->children.push_back(child); }

        inline virtual void _draw (const Background *background, const bool only_border) const {}

        inline virtual Mesh *getCollisionSpace (const Vec<3> &speed) const { return nullptr; }

        inline virtual bool _detectCollision (
            const Mesh *other,
            const Vec<3> &my_offset,
            const Vec<3> &other_offset,
            Vec<3> &point,
            const bool try_inverse = true
        ) const { return false; }

        inline bool detectCollision (
            Mesh *other,
            const Vec<3> &my_offset,
            const Vec<3> &my_speed,
            const Vec<3> &other_offset,
            const Vec<3> &other_speed,
            Vec<3> &point,
            const bool try_inverse = true
        ) {

            if (this->_detectCollision(other, my_offset, other_offset, point, try_inverse)) {
                return true;
            }

            if (my_speed) {

                const std::unique_ptr<const Mesh> my_space(this->getCollisionSpace(my_speed));

                if (my_space) {

                    if (my_space->_detectCollision(other, my_offset, other_offset, point, try_inverse)) {
                        std::cout << "first case " << this->getType() << " " << other->getType() << std::endl;
                        return true;
                    }

                    if (try_inverse && other_speed) {

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

        inline const Quaternion &getOrientation (void) const { return this->orientation; }
        inline void setOrientation (const Vec<3> &_orientation) { this->orientation = _orientation; }

        inline const Vec<3> &getPosition () const { return this->position; }
        inline void setPosition (const Vec<3> &_position) { this->position = _position; }

        inline virtual const std::string getType (void) const { return "mesh"; }
    };

    class Rectangle2D : public Mesh {

        float_max_t width, height;
        Vec<3> top_left, top_right, bottom_left, bottom_right;

    public:
        inline Rectangle2D (const Vec<3> &_position, float_max_t _width, float_max_t _height, const Quaternion _orientation = Quaternion::identity) :
            Mesh(_position, _orientation), width(_width), height(_height) {
            this->updatePositions();
        }

        void updatePositions (void) {
            const Vec<3>
                top_left = this->getPosition(),
                bottom_right = { top_left[0] + this->getWidth(), top_left[1] - this->getHeight(), top_left[2] };

            this->top_left = this->getOrientation().rotated(top_left);
            this->bottom_left = this->getOrientation().rotated({ top_left[0], bottom_right[1], top_left[2] });
            this->bottom_right = this->getOrientation().rotated(bottom_right);
            this->top_right = this->getOrientation().rotated({ bottom_right[0], top_left[1], top_left[2] });
        }

        inline float_max_t getWidth (void) const { return this->width; }
        inline float_max_t getHeight (void) const { return this->height; }

        inline void setWidth (const float_max_t _width) { this->width = _width, this->updatePositions(); }
        inline void setHeight (const float_max_t _height) { this->height = _height, this->updatePositions(); }
        inline void setOrientation (const Vec<3> &_orientation) { Mesh::setOrientation(_orientation), this->updatePositions(); }
        inline void setPosition (const Vec<3> &_position) { Mesh::setPosition(_position), this->updatePositions(); }

        inline const Vec<3> &getTopLeftPosition (void) const { return this->top_left; }
        inline const Vec<3> &getTopRightPosition (void) const { return this->top_right; }
        inline const Vec<3> &getBottomLeftPosition (void) const { return this->bottom_left; }
        inline const Vec<3> &getBottomRightPosition (void) const { return this->bottom_right; }

        void _draw (const Background *background, const bool only_border) const {

            const Vec<3>
                top_left = this->getPosition(),
                bottom_right = { top_left[0] + this->getWidth(), top_left[1] - this->getHeight(), top_left[2] };

            if (only_border) {
                Draw::begin(GL_LINE_LOOP);
            } else {
                Draw::begin(GL_QUADS);
            }

            background->apply();

            Draw::normal(Vec<3>::axisZ);
            Draw::vertex(top_left);
            Draw::vertex(top_left[0], bottom_right[1], top_left[2]);
            Draw::vertex(bottom_right);
            Draw::vertex(bottom_right[0], top_left[1], top_left[2]);

            Draw::end();
        }

        inline bool _detectCollision (
            const Mesh *other,
            const Vec<3> &my_offset,
            const Vec<3> &other_offset,
            Vec<3> &point,
            const bool try_inverse = true
        ) const {

            if (other->getType() == "rectangle2d") {
                const Rectangle2D *rect = static_cast<const Rectangle2D *>(other);
                return Mesh::collisionRectangles(
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

        float_max_t radius;
        unsigned sides;

    public:

        inline Polygon2D (const Vec<3> &_position, float_max_t _radius, unsigned _sides, const Quaternion _orientation = Quaternion::identity) :
            Mesh(_position, _orientation), radius(_radius), sides(_sides) {};

        void _draw (const Background *background, const bool only_border) const {

            const Vec<3> position = this->getPosition();
            const float_max_t step = (PI * 2.0) / static_cast<float_max_t>(this->sides);

            float_max_t ang = 0.0;

            if (only_border) {
                Draw::begin(GL_LINE_LOOP);
            } else {
                Draw::begin(GL_POLYGON);
            }

            background->apply();

            Draw::normal(Vec<3>::axisZ);

            for (unsigned i = 0; i < this->sides; i++) {

                Draw::vertex(position[0] + this->getRadius() * std::cos(ang), position[1] + this->getRadius() * std::sin(ang), position[2]);
                ang += step;

            }

            Draw::end();
        }

        inline float_max_t getRadius (void) const { return this->radius; }
        inline void setRadius (const float_max_t _radius) { this->radius = _radius; }

        Mesh *getCollisionSpace (const Vec<3> &speed) const {

            if (speed.length2() > (this->getRadius() * this->getRadius())) {
                const float_max_t speed_angle = std::atan2(speed[1], speed[0]);

                const Vec<3>
                    difference = {
                        this->getRadius() * std::cos(speed_angle + DEG90),
                        this->getRadius() * std::sin(speed_angle + DEG90),
                        0.0
                    },
                    top_position = this->getPosition() + difference;

                return new Rectangle2D(top_position, speed.length(), this->getRadius() * 2.0, Quaternion::difference(speed.normalized(), Vec<3>::axisX));
            } else {
                return nullptr;
            }
        }

        bool _detectCollision (
            const Mesh *other,
            const Vec<3> &my_offset,
            const Vec<3> &other_offset,
            Vec<3> &point,
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
        Sphere2D (const Vec<3> &_position, const float_max_t _radius) :
            Polygon2D (_position, _radius, _radius * 20, Quaternion::identity) {}

        inline const std::string getType (void) const { return "sphere2d"; }
    };
};

#endif
