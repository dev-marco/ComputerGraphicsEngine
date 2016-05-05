#ifndef SRC_ENGINE_MESH_H_
#define SRC_ENGINE_MESH_H_

#include <valarray>
#include <vector>
#include <array>
#include <memory>
#include <iostream>
#include <numeric>
#include <cmath>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "background.h"

namespace Engine {
    class Mesh {

        std::valarray<double> position;
        std::vector<std::shared_ptr<Mesh>> children;

    public:

        enum Axis {

            AXIS_X,
            AXIS_Y,
            AXIS_Z

        };

        static constexpr long double
            PI = 3.141592653589793238462643383279502884L,
            DEG45 = PI / 4.0,
            DEG90 = PI / 2.0,
            DEG135 = DEG90 + DEG45,
            DEG225 = -DEG135,
            DEG270 = -DEG90,
            DEG315 = -DEG45;

        inline static double raySize (const std::valarray<double> &ray) {
            return std::sqrt((ray * ray).sum());
        }

        inline static std::valarray<double> resizeRay (const std::valarray<double> &ray, double vector_size, double new_size) {
            return ray * (new_size / vector_size);
        }

        inline static std::valarray<double> resizeRay (const std::valarray<double> &ray, double new_size) {
            return Mesh::resizeRay(ray, Mesh::raySize(ray), new_size);
        }

        static std::valarray<double> rotateRay (const std::valarray<double> &ray, double angle, Axis axis = Mesh::Axis::AXIS_Z) {

            const double
                sin_angle = std::sin(angle),
                cos_angle = std::cos(angle);

            switch (axis) {
                case Axis::AXIS_X:
                    return { ray[0], cos_angle * ray[1] - sin_angle * ray[2], sin_angle * ray[1] + cos_angle * ray[2] };
                case Axis::AXIS_Y:
                    return { cos_angle * ray[0] + sin_angle * ray[2], ray[1], cos_angle * ray[2] - sin_angle * ray[0] };
                default:
                    return { cos_angle * ray[0] - sin_angle * ray[1], sin_angle * ray[0] + cos_angle * ray[1], ray[2] };
            }
        }

        inline static std::valarray<double> rayEquation (
            const std::valarray<double> &ray_start,
            const std::valarray<double> &ray_end
        ) {
            double b, m = (ray_end[1] - ray_start[1]) / (ray_end[0] - ray_start[0]);
            b = ray_start[1] - m * ray_start[0];
            return { m, -1.0, b };
        }

        inline static double distance2D (std::valarray<double> point_1, std::valarray<double> point_2) {
            std::valarray<double> diff = point_1 - point_2;
            return std::sqrt((diff * diff).sum());
        }

        static double distanceRayToPoint2D (
            const std::valarray<double> &ray_start,
            const std::valarray<double> &ray_end,
            const std::valarray<double> &point,
            std::valarray<double> &near_point,
            bool infinite = false
        ) {
            std::valarray<double> delta_ray = ray_end - ray_start;
            double param, length_pow = (delta_ray * delta_ray).sum();

            if (length_pow == 0.0) {
                return distance2D(point, ray_start);
            }

            param = ((point - ray_start) * delta_ray).sum() / length_pow;

            if (!infinite) {
                if (param < 0.0) {
                    param = 0.0;
                } else if (param > 1.0) {
                    param = 1.0;
                }
            }

            near_point = ray_start + param * delta_ray;

            return distance2D(point, near_point);
        }

        inline static bool collisionCircles2D (
            std::valarray<double> position_1,
            double radius_1,
            std::valarray<double> position_2,
            double radius_2
        ) { return Mesh::distance2D(position_1, position_2) <= (radius_1 + radius_2); }

        // inline static bool collisionRectangles2D (
        //     std::valarray<double> position_1,
        //     double width_1,
        //     double height_1,
        //     std::valarray<double> position_2,
        //     double width_2,
        //     double height_2
        // ) { return position_1[0] < (position_2[0] + width_2 ) && (position_1[0] + width_1 ) > position_2[0] &&
        //            position_1[1] < (position_2[1] + height_2) && (position_1[1] + height_1) > position_2[1]; }

        inline static bool collisionRayCircle2D (
            std::valarray<double> ray_start,
            std::valarray<double> ray_end,
            std::valarray<double> circle_center,
            double circle_radius,
            std::valarray<double> &near_point,
            bool infinite = false
        ) { return Mesh::distanceRayToPoint2D(ray_start, ray_end, circle_center, near_point, infinite) <= circle_radius; }

        static bool collisionRectangleCircle2D (
            const std::valarray<double> &rect_top_left,
            const std::valarray<double> &rect_bottom_left,
            const std::valarray<double> &rect_bottom_right,
            const std::valarray<double> &rect_top_right,
            double rect_angle,
            std::valarray<double> circle_center,
            double circle_radius,
            std::valarray<double> &near_point
        ) {

            if (Mesh::collisionRayCircle2D(rect_top_left, rect_top_right, circle_center, circle_radius, near_point) ||
                Mesh::collisionRayCircle2D(rect_top_right, rect_bottom_right, circle_center, circle_radius, near_point) ||
                Mesh::collisionRayCircle2D(rect_bottom_left, rect_bottom_right, circle_center, circle_radius, near_point) ||
                Mesh::collisionRayCircle2D(rect_top_left, rect_bottom_left, circle_center, circle_radius, near_point)) {
                return true;
            }

            const std::valarray<double>
                rot_rect_top_left = Mesh::rotateRay(rect_top_left, -rect_angle),
                rot_rect_bottom_right = Mesh::rotateRay(rect_bottom_right, -rect_angle),
                rot_circle_center = Mesh::rotateRay(circle_center, -rect_angle);

            if (
                rot_rect_top_left[0] <= rot_circle_center[0] && rot_circle_center[0] <= rot_rect_bottom_right[0] &&
                rot_rect_top_left[1] <= rot_circle_center[1] && rot_circle_center[1] <= rot_rect_bottom_right[1]
            ) {
                near_point = circle_center;
                return true;
            }

            return false;
        }

        Mesh (const std::valarray<double> &_position = { 0.0, 0.0, 0.0 }) :
            position(_position) {};

        virtual ~Mesh () {}

        void draw (const std::valarray<double> &offset, const Background *background, bool only_border = false) const {

            this->_draw(offset, background, only_border);

            if (!this->children.empty()) {
                std::valarray<double> position = offset + this->getPosition();

                for (const auto &mesh : this->children) {
                    mesh->draw(position, background, only_border);
                }
            }
        }

        inline void addChild (std::shared_ptr<Mesh> child) { this->children.push_back(child); }

        inline const std::valarray<double> &getPosition () const { return this->position; }

        inline void setPosition (const std::valarray<double> _position) { this->position = std::valarray<double>(_position); }

        inline virtual void _draw (const std::valarray<double> &offset, const Background *background, bool only_border) const {}

        virtual bool detectCollision (
            const Mesh *other,
            const std::valarray<double> &my_position,
            const std::valarray<double> &other_position,
            std::valarray<double> &point,
            bool try_inverse = true
        ) const {
            if (try_inverse) {
                return other->detectCollision(this, other_position, my_position, point, false);
            }
            return false;
        }

        inline virtual std::string getType () const { return "mesh"; }
    };

    class Rectangle2D : public Mesh {

        double width, height, angle;

        std::valarray<double> top_right, bottom_left, bottom_right;

    public:
        inline Rectangle2D (const std::valarray<double> &_position, double _width, double _height, double _angle = 0.0) :
            Mesh(_position), width(_width), height(_height), angle(_angle) {
            this->updatePositions();
        }

        void updatePositions (void) {
            const double
                height_angle = this->getAngle() - DEG90;

            const std::valarray<double>
                delta_height = { this->getHeight() * std::cos(height_angle), this->getHeight() * std::sin(height_angle), 0.0 };

            this->top_right = this->getTopLeftPosition() + std::valarray<double>(
                { this->getWidth() * std::cos(this->getAngle()), this->getWidth() * std::sin(this->getAngle()), 0.0 }
            );

            this->bottom_left = this->getTopLeftPosition() + delta_height;
            this->bottom_right = this->getTopRightPosition() + delta_height;

        }

        double getWidth (void) const { return this->width; }
        double getHeight (void) const { return this->height; }
        double getAngle (void) const { return this->angle; }

        inline void setWidth (double _width) { this->width = _width, this->updatePositions(); }
        inline void setHeight (double _height) { this->height = _height, this->updatePositions(); }
        inline void setAngle (double _angle) { this->angle = _angle, this->updatePositions(); }

        inline const std::valarray<double> &getTopLeftPosition (void) const { return this->getPosition(); }
        inline const std::valarray<double> &getTopRightPosition (void) const { return this->top_right; }
        inline const std::valarray<double> &getBottomLeftPosition (void) const { return this->bottom_left; }
        inline const std::valarray<double> &getBottomRightPosition (void) const { return this->bottom_right; }

        void _draw (const std::valarray<double> &offset, const Background *background, bool only_border) const {

            const std::valarray<double>
                top_left = this->getTopLeftPosition() + offset,
                bottom_left = this->getBottomLeftPosition() + offset,
                bottom_right = this->getBottomRightPosition() + offset,
                top_right = this->getTopRightPosition() + offset;

            if (only_border) {
                glBegin(GL_LINE_LOOP);
            } else {
                glBegin(GL_TRIANGLE_FAN);
            }

            background->apply();

            glVertex3d(top_left[0], top_left[1], top_left[2]);
            glVertex3d(bottom_left[0], bottom_left[1], bottom_left[2]);
            glVertex3d(bottom_right[0], bottom_right[1], bottom_right[2]);
            glVertex3d(top_right[0], top_right[1], top_right[2]);

            glEnd();

        }

        inline std::string getType (void) const { return "rectangle2d"; }

    };

    class Polygon2D : public Mesh {

        double radius, angle;
        int sides;

    public:

        inline Polygon2D (const std::valarray<double> &_position, double _radius, int _sides, double _angle = 0.0) :
            Mesh(_position), radius(_radius), angle(_angle), sides(_sides) {};

        void _draw (const std::valarray<double> &offset, const Background *background, bool only_border) const {

            std::valarray<double> position = offset + this->getPosition();

            double step = (Polygon2D::PI * 2.0) / static_cast<double>(this->sides);
            if (only_border) {
                glBegin(GL_LINE_LOOP);
            } else {
                glBegin(GL_TRIANGLE_FAN);
            }

            background->apply();

            for (int i = 0; i < this->sides; i++) {

                double ang = i * step + this->getAngle();
                glVertex3d(position[0] + this->getRadius() * std::cos(ang), position[1] + this->getRadius() * std::sin(ang), position[2]);

            }

            glEnd();

        }

        inline double getRadius (void) const { return this->radius; }
        inline double getAngle (void) const { return this->angle; }

        inline void setRadius (double _radius) { this->radius = _radius; }

        inline virtual std::string getType () const { return "polygon2d"; }

        Mesh *getCollisionRectangle (const std::valarray<double> &speed) const {

            const double angle = std::atan2(speed[1], speed[0]);

            const std::valarray<double>
                difference = {
                    this->getRadius() * static_cast<double>(std::cos(angle - DEG90)),
                    this->getRadius() * static_cast<double>(std::sin(angle - DEG90))
                },
                top_position = this->getPosition() + difference;

            return new Rectangle2D(top_position, Mesh::raySize(speed), this->getRadius() * 2.0, angle);
        }

        bool detectCollision (
            const Mesh *other,
            const std::valarray<double> &my_offset,
            const std::valarray<double> &other_offset,
            std::valarray<double> &point,
            bool try_inverse = true
        ) const {

            if (other->getType() == "polygon2d" || other->getType() == "sphere2d") {
                const Polygon2D *poly = static_cast<const Polygon2D *>(other);
                point = (this->getPosition() + other->getPosition()) * 0.5;
                return Mesh::collisionCircles2D(my_offset + this->getPosition(), this->getRadius(), other_offset + other->getPosition(), poly->getRadius());
            } else if (other->getType() == "rectangle2d") {
                const Rectangle2D *rect = static_cast<const Rectangle2D *>(other);
                return Mesh::collisionRectangleCircle2D(
                    other_offset + rect->getTopLeftPosition(),
                    other_offset + rect->getBottomLeftPosition(),
                    other_offset + rect->getBottomRightPosition(),
                    other_offset + rect->getTopRightPosition(),
                    rect->getAngle(),
                    my_offset + this->getPosition(),
                    this->getRadius(),
                    point
                );
            }
            return Mesh::detectCollision(other, my_offset, other_offset, point, try_inverse);
        }

    };

    class Sphere2D : public Polygon2D {
    public:
        Sphere2D (const std::valarray<double> &_position, double _radius) :
            Polygon2D (_position, _radius, 360, 0.0) {}

        inline virtual std::string getType () const { return "sphere2d"; }
    };
};

#endif
