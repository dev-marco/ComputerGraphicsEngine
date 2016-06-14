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

        inline static std::array<std::array<Vec<3>, 2>, 3> edgesTriangle (
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

        inline static std::array<std::array<Vec<3>, 2>, 4> edgesRectangle (
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

        inline static float_max_t areaRectangle (
            const Vec<3> &rect_top_left,
            const Vec<3> &rect_bottom_left,
            const Vec<3> &rect_bottom_right
        ) {
            return std::sqrt((rect_bottom_left - rect_top_left).length2() * (rect_bottom_right - rect_bottom_left).length2());
        }

// -----------------------------------------------------------------------------

        static float_max_t distancePointRay (
            const Vec<3> &point,
            const Vec<3> &ray_start,
            const Vec<3> &ray_end,
            Vec<3> &near_point
        ) {
            const Vec<3> delta_ray = ray_end - ray_start;
            const float_max_t length_2 = delta_ray.length2();

            if (length_2 == 0.0) {
                near_point = ray_start;
            } else {
                const float_max_t param = clamp(((point - ray_start) * delta_ray).sum() / length_2, 0.0, 1.0);
                near_point = ray_start + (param * delta_ray);
            }

            return point.distance(near_point);
        }

        static float_max_t distancePointSphere (
            const Vec<3> &point,
            const Vec<3> &sphere_center,
            float_max_t sphere_radius
        ) {
            float_max_t d = point.distance(sphere_center) - sphere_radius;
            if (d > 0.0) {
                return d;
            }
            return 0.0;
        }

        // NOTE http://liris.cnrs.fr/Documents/Liris-1297.pdf
        static float_max_t distancePointCylinder (
            const Vec<3> &point,
            const Vec<3> &cylinder_start,
            const Vec<3> &cylinder_end,
            float_max_t cylinder_radius
        ) {
            const Vec<3>
                diff = cylinder_end - cylinder_start,
                c = cylinder_start.lerped(cylinder_end, 0.5);
            float_max_t
                l = diff.length(),
                x = std::abs((c - point).dot(diff / l)),
                y2 = c.distance2(point) - (x * x);

            x -= l / 2.0;

            if (x < 0) {
                if (y2 < (cylinder_radius * cylinder_radius)) {
                    return 0.0;
                } else {
                    return std::sqrt(y2) - cylinder_radius;
                }
            }
            if (y2 < (cylinder_radius * cylinder_radius)) {
                return x;
            } else {
                float_max_t p = std::sqrt(y2) - cylinder_radius;
                return std::sqrt(p * p + x * x);
            }
        }

        static float_max_t distancePointPlane (
            const Vec<3> &point,
            const Vec<3> &plane_normal,
            const Vec<3> &plane_point,
            Vec<3> &near_point
        ) {
            near_point = point + plane_normal * ((plane_normal.dot(plane_point) - plane_normal.dot(point)) / plane_normal.length2());
            return near_point.distance(point);
        }

// -----------------------------------------------------------------------------

        static float_max_t distanceSphereSphere (
            const Vec<3> &center_1,
            float_max_t radius_1,
            const Vec<3> &center_2,
            float_max_t radius_2
        ) {
            float_max_t d = distancePointSphere(center_1, center_2, radius_2) - radius_1;
            if (d > 0.0) {
                return d;
            }
            return 0.0;
        }

        static float_max_t distanceSphereCylinder (
            const Vec<3> &sphere_center,
            float_max_t sphere_radius,
            const Vec<3> &cylinder_start,
            const Vec<3> &cylinder_end,
            float_max_t cylinder_radius
        ) {
            float_max_t d = distancePointCylinder(sphere_center, cylinder_start, cylinder_end, cylinder_radius) - sphere_radius;
            if (d > 0.0) {
                return d;
            }
            return 0.0;
        }

        static float_max_t distanceSpherePlane (
            const Vec<3> &sphere_center,
            float_max_t sphere_radius,
            const Vec<3> &plane_normal,
            const Vec<3> &plane_point,
            Vec<3> &near_point
        ) {
            float_max_t d = distancePointPlane(sphere_center, plane_normal, plane_point, near_point) - sphere_radius;
            if (d > 0.0) {
                return d;
            }
            return 0.0;
        }

// -----------------------------------------------------------------------------

        // NOTE Real-Time Collision Detection
        static float_max_t distanceRayRay (
            const Vec<3> &ray_1_start,
            const Vec<3> &ray_1_end,
            const Vec<3> &ray_2_start,
            const Vec<3> &ray_2_end,
            Vec<3> &ray_start,
            Vec<3> &ray_end
        );

// -----------------------------------------------------------------------------

        inline static bool intersectionPointTriangle2D (
            const Vec<3> &point,
            const Vec<3> &tri_point_1,
            const Vec<3> &tri_point_2,
            const Vec<3> &tri_point_3
        ) {
            return intersectionPointConvexPolygon2D(point, edgesTriangle(tri_point_1, tri_point_2, tri_point_3));
        };

        inline static bool intersectionPointRectangle2D (
            const Vec<3> &point,
            const Vec<3> &rect_top_left,
            const Vec<3> &rect_bottom_left,
            const Vec<3> &rect_bottom_right,
            const Vec<3> &rect_top_right
        ) {
            return intersectionPointConvexPolygon2D(point, edgesRectangle(rect_top_left, rect_bottom_left, rect_bottom_right, rect_top_right));
        }

// -----------------------------------------------------------------------------

        inline static bool intersectionRaySphere (
            const Vec<3> &ray_start,
            const Vec<3> &ray_end,
            const Vec<3> &circle_center,
            const float_max_t circle_radius,
            Vec<3> &near_point
        ) { return distancePointRay(circle_center, ray_start, ray_end, near_point) <= circle_radius; }

        // NOTE Real-Time Collision Detection : 180
        inline static bool intersectionRayBox (
            const Vec<3> &ray_start,
            const Vec<3> &ray_end,
            const Vec<3> &box_min,
            const Vec<3> &box_max,
            Vec<3> &near_point,
            float_max_t tmin = -std::numeric_limits<float_max_t>::infinity(),
            float_max_t tmax = std::numeric_limits<float_max_t>::infinity()
        ) {
            const Vec<3> diff = ray_end - ray_start;
            for (unsigned i = 0; i < 3; ++i) {
                if (std::abs(diff[i]) < EPSILON) {
                    if (ray_start[i] < box_min[i] || ray_start[i] > box_max[i]) {
                        return 0;
                    }
                } else {
                    float_max_t
                        ood = 1.0 / diff[i],
                        t1 = (box_min[i] - ray_start[i]) * ood,
                        t2 = (box_max[i] - ray_start[i]) * ood;
                    if (t1 > t2) {
                        std::swap(t1, t2);
                    }
                    if (t1 > tmin) {
                        tmin = t1;
                    }
                    if (t2 > tmax) {
                        tmax = t2;
                    }
                    if (tmin > tmax) {
                        return false;
                    }
                }
            }
            near_point = ray_start + diff * tmin;
            return true;
        }

        // NOTE Real-Time Collision Detection : 236

// -----------------------------------------------------------------------------

        template <typename T, typename = typename T::iterator>
        static constexpr bool intersectionPointConvexPolygon2D (
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

        inline static bool intersectionSphereSphere (
            const Vec<3> &position_1,
            const float_max_t radius_1,
            const Vec<3> &position_2,
            const float_max_t radius_2
        ) {
            const float_max_t center_distance = radius_1 + radius_2;
            return position_1.distance2(position_2) <= (center_distance * center_distance);
        }

        static bool intersectionRectangleRectangle (
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

        inline static bool intersectionRectangleCircle2D (
            const Vec<3> &rect_top_left,
            const Vec<3> &rect_bottom_left,
            const Vec<3> &rect_bottom_right,
            const Vec<3> &rect_top_right,
            const Vec<3> &circle_center,
            const float_max_t circle_radius,
            Vec<3> &near_point
        ) {

            if (intersectionRaySphere(rect_top_left, rect_top_right, circle_center, circle_radius, near_point) ||
                intersectionRaySphere(rect_top_right, rect_bottom_right, circle_center, circle_radius, near_point) ||
                intersectionRaySphere(rect_bottom_left, rect_bottom_right, circle_center, circle_radius, near_point) ||
                intersectionRaySphere(rect_top_left, rect_bottom_left, circle_center, circle_radius, near_point)) {
                return true;
            }

            if (intersectionPointRectangle2D(rect_top_left, rect_bottom_left, rect_bottom_right, rect_top_right, circle_center)) {
                near_point = circle_center;
                return true;
            }

            return false;
        }
    private:

            Vec<3> position;
            Quaternion orientation;
            std::vector<Mesh *> children;
            Background *background;

    public:

        Mesh (const Vec<3> &_position = Vec<3>::zero, const Quaternion &_orientation = Quaternion::identity, Background *_background = nullptr) :
            position(_position), orientation(_orientation), background(_background) {};

        virtual ~Mesh () {}

        virtual void draw(const bool only_border = false) const final;
        inline virtual void _draw (const bool only_border) const {}

        inline virtual Mesh *getCollisionSpace (const Vec<3> &speed) const { return nullptr; }

        inline virtual bool detectCollision (
            Mesh *other,
            const Vec<3> &my_offset,
            const Vec<3> &my_speed,
            const Vec<3> &other_offset,
            const Vec<3> &other_speed,
            Vec<3> &point,
            const bool try_inverse = true
        ) final {

            if (this->_detectCollision(other, my_offset, other_offset, point, try_inverse)) {
                return true;
            }

            if (my_speed) {

                const std::unique_ptr<const Mesh> my_space(this->getCollisionSpace(my_speed));

                if (my_space) {

                    if (my_space->_detectCollision(other, my_offset, other_offset, point, try_inverse)) {
                        return true;
                    }

                    if (try_inverse && other_speed) {

                        const std::unique_ptr<const Mesh> other_space(other->getCollisionSpace(other_speed));

                        if (
                            other_space &&
                            my_space->_detectCollision(other_space.get(), my_offset, other_offset, point, try_inverse)
                        ) {
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

        inline virtual bool _detectCollision (
            const Mesh *other,
            const Vec<3> &my_offset,
            const Vec<3> &other_offset,
            Vec<3> &point,
            const bool try_inverse = true
        ) const { return false; }

        inline const Quaternion &getOrientation (void) const { return this->orientation; }
        inline virtual void setOrientation (const Quaternion &_orientation) { this->orientation = _orientation; }

        inline const Vec<3> &getPosition () const { return this->position; }
        inline virtual void setPosition (const Vec<3> &_position) { this->position = _position; }

        inline Background *getBackground (void) const { return this->background; }
        inline void setBackground (Background *_background) { this->background = _background; }

        inline const std::vector<Mesh *> &getChildren (void) const { return this->children; }
        inline virtual void addChild (Mesh *child) { this->children.push_back(child); }

        virtual void debugInfo (std::ostream &out, const std::string shift = "") const {
            out << shift << "Mesh Type: " << this->getType() << std::endl;
            out << shift << "Mesh Position: " << this->getPosition() << std::endl;
            out << shift << "Mesh Children:" << std::endl;
            for (const auto &child : this->getChildren()) {
                child->debugInfo(out, shift + ' ');
            }
            out << std::endl;
        }
        inline virtual const std::string getType (void) const { return "mesh"; }
    };

    class Rectangle2D : public Mesh {

        float_max_t width, height;
        Vec<3> top_left, top_right, bottom_left, bottom_right;

    public:
        inline Rectangle2D (const Vec<3> &_position, float_max_t _width, float_max_t _height, const Quaternion _orientation = Quaternion::identity, Background *_background = nullptr) :
            Mesh(_position, _orientation, _background), width(_width), height(_height) {
            this->updatePositions();
        }

        inline void updatePositions (void) {
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
        inline void setOrientation (const Quaternion &_orientation) override { Mesh::setOrientation(_orientation), this->updatePositions(); }
        inline void setPosition (const Vec<3> &_position) { Mesh::setPosition(_position), this->updatePositions(); }

        inline const Vec<3> &getTopLeftPosition (void) const { return this->top_left; }
        inline const Vec<3> &getTopRightPosition (void) const { return this->top_right; }
        inline const Vec<3> &getBottomLeftPosition (void) const { return this->bottom_left; }
        inline const Vec<3> &getBottomRightPosition (void) const { return this->bottom_right; }

        void _draw (const bool only_border) const override {

            const float_max_t
                width = this->getWidth(),
                height = -this->getHeight();

            if (only_border) {
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            }

            Draw::begin(this->getBackground());

            Draw::normal(Vec<3>::axisZ);

            Draw::vertex(0.0, 0.0, 0.0);
            Draw::vertex(0.0, height, 0.0);
            Draw::vertex(width, 0.0, 0.0);

            Draw::vertex(width, 0.0, 0.0);
            Draw::vertex(0.0, height, 0.0);
            Draw::vertex(width, height, 0.0);

            Draw::end();
        }

        inline bool _detectCollision (
            const Mesh *other,
            const Vec<3> &my_offset,
            const Vec<3> &other_offset,
            Vec<3> &point,
            const bool try_inverse = true
        ) const override {

            if (other->getType() == "rectangle2d") {
                const Rectangle2D *rect = static_cast<const Rectangle2D *>(other);
                return Mesh::intersectionRectangleRectangle(
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

        inline const std::string getType (void) const override { return "rectangle2d"; }

    };

    class Polygon2D : public Mesh {

        float_max_t radius, ratio_x, ratio_y;
        unsigned sides;
        std::vector<Vec<2>> vertexes;

        void updateVertexes (void) {

            const Vec<3> position = this->getPosition();
            const float_max_t
                sides = this->getSides(),
                radius = this->getRadius(),
                step = (PI * 2.0) / static_cast<float_max_t>(sides);
            float_max_t ang = 0.0;

            this->vertexes.resize(sides);

            for (Vec<2> &vertex : this->vertexes) {
                vertex[0] = position[0] + radius * std::cos(ang) * this->getRatioX();
                vertex[1] = position[1] + radius * std::sin(ang) * this->getRatioY();
                ang += step;
            }
        }

    public:

        inline Polygon2D (const Vec<3> &_position, float_max_t _radius, unsigned _sides, float_max_t _ratio_x = 1.0, float_max_t _ratio_y = 1.0, const Quaternion _orientation = Quaternion::identity, Background *_background = nullptr) :
            Mesh(_position, _orientation, _background), radius(_radius), ratio_x(_ratio_x), ratio_y(_ratio_y), sides(_sides) {
                this->updateVertexes();
            };

        void _draw (const bool only_border) const override {

            const Vec<3> position = this->getPosition();
            const std::vector<Vec<2>> vertexes = this->getVertexes();
            const float_max_t sides = this->getSides();

            unsigned j = 0;

            if (only_border) {
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            }

            Draw::begin(this->getBackground());

            Draw::normal(Vec<3>::axisZ);

            for (unsigned i = 1; i < sides; ++i, ++j) {
                Draw::vertex(vertexes[j][0], vertexes[j][1], position[2]);
                Draw::vertex(position);
                Draw::vertex(vertexes[i][0], vertexes[i][1], position[2]);
            }

            Draw::vertex(vertexes[j][0], vertexes[j][1], position[2]);
            Draw::vertex(position);
            Draw::vertex(vertexes[0][0], vertexes[0][1], position[2]);

            Draw::end();
        }

        inline float_max_t getRadius (void) const { return this->radius; }
        inline void setRadius (const float_max_t _radius) { this->radius = _radius, this->updateVertexes(); }

        inline float_max_t getRatioX (void) const { return this->ratio_x; }
        inline void setRatioX (const float_max_t _ratio_x) { this->ratio_x = _ratio_x, this->updateVertexes(); }

        inline float_max_t getRatioY (void) const { return this->ratio_y; }
        inline void setRatioY (const float_max_t _ratio_y) { this->ratio_y = _ratio_y, this->updateVertexes(); }

        inline unsigned getSides (void) const { return this->sides; }
        inline void setSides (const unsigned _sides) { this->sides = _sides, this->updateVertexes(); }

        inline const std::vector<Vec<2>> &getVertexes (void) const { return this->vertexes; }

        Mesh *getCollisionSpace (const Vec<3> &speed) const override {

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
        ) const override {

            if (other->getType() == "polygon2d" || other->getType() == "sphere2d") {
                const Polygon2D *poly = static_cast<const Polygon2D *>(other);
                if (Mesh::intersectionSphereSphere(my_offset + this->getPosition(), this->getRadius(), other_offset + other->getPosition(), poly->getRadius())) {
                    point = (this->getPosition() + other->getPosition()) * 0.5;
                    return true;
                }
            } else if (other->getType() == "rectangle2d") {
                const Rectangle2D *rect = static_cast<const Rectangle2D *>(other);
                return Mesh::intersectionRectangleCircle2D(
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

        inline const std::string getType (void) const override { return "polygon2d"; }

    };

    class Sphere2D : public Polygon2D {
    public:
        Sphere2D (const Vec<3> &_position, float_max_t _radius, Background *_background) :
            Polygon2D(_position, _radius, 20, 1.0, 1.0, Quaternion::identity, _background) {}

        inline const std::string getType (void) const override { return "sphere2d"; }
    };

    class Ellipse2D : public Polygon2D {
    public:
        Ellipse2D (const Vec<3> &_position, float_max_t _radius, float_max_t _ratio_x, float_max_t _ratio_y, Background *_background) :
            Polygon2D(_position, _radius, 20, _ratio_x, _ratio_y, Quaternion::identity, _background) {}

        inline const std::string getType (void) const override { return "ellipse2d"; }
    };

    class Cone : public Mesh {

        Vec<3> end;
        float_max_t base_radius, top_radius, height;

    public:
        Cone (const Vec<3> &_start, const Vec<3> &_end, float_max_t _base_radius, float_max_t _top_radius, Background *_background = nullptr) :
            Mesh(_start, Quaternion::identity, _background), end(_end), base_radius(_base_radius), top_radius(_top_radius), height(_start.distance(_end)) {
                this->setOrientation(Quaternion::difference(Vec<3>::axisZ, this->getEnd()));
        }

        Cone (const Vec<3> &_start, const Quaternion &_orientation, float_max_t _base_radius, float_max_t _top_radius, const float_max_t _height, Background *_background) :
            Mesh(_start, _orientation, _background), end(_orientation.rotated(_start + Vec<3>({ 0.0, 0.0, _height }))), base_radius(_base_radius), top_radius(_top_radius), height(_height) {}

        inline float_max_t getBaseRadius (void) const { return this->base_radius; }
        inline float_max_t getTopRadius (void) const { return this->top_radius; }

        inline virtual void setBaseRadius (float_max_t _base_radius) { this->base_radius = _base_radius; }
        inline virtual void setTopRadius (float_max_t _top_radius) { this->top_radius = _top_radius; }

        inline float_max_t getHeight (void) const { return this->height; }

        inline const Vec<3> &getStart (void) const { return this->getPosition(); }
        inline const Vec<3> &getEnd (void) const { return this->end; }

        void _draw (const bool only_border) const {
            this->getBackground()->apply();
            auto quad = gluNewQuadric();
            gluCylinder(quad, this->getBaseRadius(), this->getTopRadius(), this->getHeight(), 10, 10);
            gluDeleteQuadric(quad);
        }

        inline const std::string getType (void) const override { return "cone"; }
    };

    class Cylinder : public Cone {

    public:
        Cylinder (const Vec<3> &_start, const Vec<3> &_end, float_max_t _radius, Background *_background = nullptr) :
            Cone(_start, _end, _radius, _radius, _background) {}

        Cylinder (const Vec<3> &_start, const Quaternion &_orientation, float_max_t _radius, float_max_t _height, Background *_background = nullptr) :
            Cone(_start, _orientation, _radius, _radius, _height, _background) {}

        inline float_max_t getRadius (void) const { return this->getBaseRadius(); }

        inline void setRadius (float_max_t _radius) {
            Cone::setBaseRadius(_radius), Cone::setTopRadius(_radius);
        }

        inline void setBaseRadius (float_max_t _radius) { this->setRadius(_radius); }
        inline void setTopRadius (float_max_t _radius) { this->setRadius(_radius); }

        inline const std::string getType (void) const override { return "cylinder"; }
    };

    class Sphere3D : public Mesh {

        float_max_t radius;

        static void recursiveTriangle(
            const Vec<3> &a,
            const Vec<3> &b,
            const Vec<3> &c,
            const float_max_t radius,
            const unsigned steps
        ) {

            if (steps == 0) {
                Draw::normal(a);
                Draw::vertex(a[0] * radius, a[1] * radius, a[2] * radius);

                Draw::normal(b);
                Draw::vertex(b[0] * radius, b[1] * radius, b[2] * radius);

                Draw::normal(c);
                Draw::vertex(c[0] * radius, c[1] * radius, c[2] * radius);
            } else {
                const unsigned next_step = steps - 1;
                const Vec<3>
                    ab = ((a + b) * 0.5).normalized(),
                    ac = ((a + c) * 0.5).normalized(),
                    bc = ((b + c) * 0.5).normalized();

                recursiveTriangle( a, ab, ac, radius, next_step);
                recursiveTriangle( b, bc, ab, radius, next_step);
                recursiveTriangle( c, ac, bc, radius, next_step);
                recursiveTriangle(ab, bc, ac, radius, next_step);
            }
        }

    public:
        Sphere3D (const Vec<3> &_position, const float_max_t _radius, Background *_background = nullptr) :
            Mesh(_position, Quaternion::identity, _background), radius(_radius) {};

        float_max_t getRadius (void) const { return this->radius; }
        void setRadius (float_max_t _radius) { this->radius = _radius; }

        void _draw (const bool only_border) const override {

            constexpr float_max_t
                X = 0.525731112119133606,
                Z = 0.850650808352039932;

            static Vec<3> vdata[12] = {
                {  -X, 0.0,   Z }, {   X, 0.0,   Z }, {  -X, 0.0,  -Z },
                {   X, 0.0,  -Z }, { 0.0,   Z,   X }, { 0.0,   Z,  -X },
                { 0.0,  -Z,   X }, { 0.0,  -Z,  -X }, {   Z,   X, 0.0 },
                {  -Z,   X, 0.0 }, {   Z,  -X, 0.0 }, {  -Z,  -X, 0.0 }
            };

            constexpr unsigned tindices[20][3] = {
                {  0,  4,  1 }, {  0,  9,  4 }, {  9,  5,  4 }, {  4,  5,  8 },
                {  4,  8,  1 }, {  8, 10,  1 }, {  8,  3, 10 }, {  5,  3,  8 },
                {  5,  2,  3 }, {  2,  7,  3 }, {  7, 10,  3 }, {  7,  6, 10 },
                {  7, 11,  6 }, { 11,  0,  6 }, {  0,  1,  6 }, {  6,  1, 10 },
                {  9,  0, 11 }, {  9, 11,  2 }, {  9,  2,  5 }, {  7,  2, 11 }
            }, steps = 1;

            if (only_border) {
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            }

            Draw::begin(this->getBackground());

            for (const auto &indices : tindices) {
                recursiveTriangle(vdata[indices[0]], vdata[indices[1]], vdata[indices[2]], this->getRadius(), steps);
            }

            Draw::end();

        }

        inline const std::string getType (void) const override { return "sphere3d"; }
    };
};

#endif
