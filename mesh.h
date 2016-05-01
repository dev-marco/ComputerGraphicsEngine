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

class Mesh {

    std::valarray<double> position;
    std::vector<std::shared_ptr<Mesh>> children;

public:

    static constexpr long double PI = 3.141592653589793238462643383279502884L;

    static std::valarray<double> rayEquation (
        const std::valarray<double> &ray_start,
        const std::valarray<double> &ray_end
    ) {
        double b, m = (ray_end[1] - ray_start[1]) / (ray_end[0] - ray_start[0]);
        b = ray_start[1] - m * ray_start[0];
        return { m, -1, b };
    }

    static double distance2D (std::valarray<double> point_1, std::valarray<double> point_2) {
        std::valarray<double> diff = point_1 - point_2;
        diff *= diff;
        return std::sqrt(diff.sum());
    }

    static double distanceRayToPoint2D (
        const std::valarray<double> &ray_start,
        const std::valarray<double> &ray_end,
        const std::valarray<double> &point,
        std::valarray<double> &near_point,
        bool infinite = false
    ) {
        std::valarray<double> delta_ray = ray_end - ray_start;
        double length_pow = std::pow(ray_start - ray_end, 2).sum();

        if (length_pow == 0.0) {
            return distance2D(point, ray_start);
        }

        std::valarray<double> point_to_start = point - ray_start;
        double param = std::inner_product(begin(point_to_start), end(point_to_start), begin(std::valarray<double>(ray_end - ray_start)), 0.0) / length_pow;

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

    inline static bool collisionRectangles2D (
        std::valarray<double> position_1,
        double width_1,
        double height_1,
        std::valarray<double> position_2,
        double width_2,
        double height_2
    ) { return position_1[0] < (position_2[0] + width_2 ) && (position_1[0] + width_1 ) > position_2[0] &&
               position_1[1] < (position_2[1] + height_2) && (position_1[1] + height_1) > position_2[1]; }

    inline static bool collisionRayCircle2D (
        std::valarray<double> ray_start,
        std::valarray<double> ray_end,
        std::valarray<double> circle_center,
        double circle_radius,
        std::valarray<double> &near_point,
        bool infinite = false
    ) { return Mesh::distanceRayToPoint2D(ray_start, ray_end, circle_center, near_point, infinite) <= circle_radius; }

    static bool collisionRectangleCircle2D (
        std::valarray<double> rect_top_left,
        double rect_width,
        double rect_height,
        std::valarray<double> circle_center,
        double circle_radius
    ) {
        std::valarray<double> rect_top_right = { rect_top_left[0] + rect_width, rect_top_left[1] },
                              rect_bottom_right = { rect_top_left[0] + rect_width, rect_top_left[1] + rect_height },
                              rect_bottom_left = { rect_top_left[0], rect_top_left[1] + rect_height };

        std::valarray<double> near_point;

        if (Mesh::collisionRayCircle2D(rect_top_left, rect_top_right, circle_center, circle_radius, near_point) ||
            Mesh::collisionRayCircle2D(rect_top_right, rect_bottom_right, circle_center, circle_radius, near_point) ||
            Mesh::collisionRayCircle2D(rect_bottom_left, rect_bottom_right, circle_center, circle_radius, near_point) ||
            Mesh::collisionRayCircle2D(rect_top_left, rect_bottom_left, circle_center, circle_radius, near_point)) {
            return true;
        }

        if (
            rect_top_left[0] <= circle_center[0] && circle_center[0] <= rect_bottom_right[0] &&
            rect_top_left[1] <= circle_center[1] && circle_center[1] <= rect_bottom_right[1]
        ) {
            near_point = circle_center;
            return true;
        }

        return false;
    }

    Mesh (const std::array<double, 3> &_position = { 0.0, 0.0, 0.0 }) :
        position(_position.data(), 3) {};

    virtual ~Mesh () {}

    void draw (const std::valarray<double> &position, const Background *background, double ratio = 1.0) const {

        std::valarray<double> offset = position + this->position;

        this->_draw(offset, background, ratio);

        for (const auto &mesh : this->children) {
            mesh->draw(offset, background, ratio);
        }
    }

    inline void addChild (std::shared_ptr<Mesh> child) { this->children.push_back(child); }

    inline const std::valarray<double> &getPosition () const { return this->position; }

    inline void setPosition (const std::array<double, 3> _position) { this->position = std::valarray<double>(_position.data(), 3); }

    inline virtual void _draw (const std::valarray<double> &position, const Background *background, double ratio) const {}

    virtual bool detectCollision (const Mesh *other, const std::valarray<double> &my_position, const std::valarray<double> &other_position, bool try_inverse = true) const { return false; }

    inline virtual std::string getType () const { return "mesh"; }

};

class Polygon2D : public Mesh {

    double radius, angle;
    int sides;

public:

    inline Polygon2D (const std::array<double, 3> &_position, double _radius, int _sides, double _angle = 0.0) :
        Mesh(_position), radius(_radius), angle(_angle), sides(_sides) {};

    void _draw (const std::valarray<double> &position, const Background *background, double ratio) const {

        double step = (Polygon2D::PI * 2.0) / static_cast<double>(this->sides);
        glBegin(GL_TRIANGLE_FAN);

        background->apply();

        for (int i = 0; i < this->sides; i++) {

            double ang = i * step + this->angle;
            glVertex3d(position[0] + this->radius * cos(ang), position[1] + this->radius * sin(ang) * ratio, position[2]);

        }

        glEnd();

    }

    inline double getRadius () const { return this->radius; }

    inline void setRadius (double _radius) { this->radius = _radius; }

    inline virtual std::string getType () const { return "polygon2d"; }

    bool detectCollision (const Mesh *other, const std::valarray<double> &my_parent_pos, const std::valarray<double> &other_parent_pos, bool try_inverse = true) const {

        if (other->getType() == "polygon2d" || other->getType() == "sphere2d") {
            const Polygon2D *poly = dynamic_cast<const Polygon2D *>(other);
            if (poly) {
                return Mesh::collisionCircles2D(my_parent_pos + this->getPosition(), this->getRadius(), other_parent_pos + other->getPosition(), poly->getRadius());
            }
        } else if (try_inverse) {
            return other->detectCollision(this, other_parent_pos, my_parent_pos, false);
        }
        return Mesh::detectCollision(other, my_parent_pos, other_parent_pos, try_inverse);
    }

};

class Sphere2D : public Polygon2D {
public:
    Sphere2D (const std::array<double, 3> &_position, double _radius) :
        Polygon2D (_position, _radius, 360, 0.0) {}

    inline virtual std::string getType () const { return "sphere2d"; }
};

class Rectangle2D : public Mesh {

    double width, height;

public:

    Rectangle2D (const std::array<double, 3> &_position, double _width, double _height) :
        Mesh(_position), width(_width), height(_height) {};

    inline void _draw (const std::valarray<double> &position, const Background *background, double ratio) const {

        glBegin(GL_TRIANGLE_FAN);

        background->apply();

        glVertex3d(position[0], position[1] + height * ratio, position[2]);
        // for (double i = position[1] + height * ratio; i > position[1]; i -= 0.001) {
        //     glVertex3d(position[0], i, position[2]);
        // }

        glVertex3d(position[0], position[1], position[2]);
        for (double i = position[0]; i < position[0] + width; i += 0.001) {
            glVertex3d(i, position[1], position[2]);
        }

        glVertex3d(position[0] + width, position[1], position[2]);
        // for (double i = position[1]; i < position[1] + height; i += 0.001) {
        //     glVertex3d(position[0] + width, i, position[2]);
        // }

        glVertex3d(position[0] + width, position[1] + height * ratio, position[2]);
        for (double i = position[0] + width; i > position[0]; i -= 0.001) {
            glVertex3d(i, position[1] + height, position[2]);
        }

        glEnd();
    }

    inline double getWidth () const { return this->width; }
    inline double getHeight () const { return this->height; }

    inline void setWidth (double _width) { this->width = _width; }
    inline void setHeight (double _height) { this->height = _height; }

    inline std::string getType () const { return "rectangle2d"; }

    bool detectCollision (const Mesh *other, const std::valarray<double> &my_parent_pos, const std::valarray<double> &other_parent_pos, bool try_inverse = true) const {
        if (other->getType() == "rectangle2d") {
            const Rectangle2D *rect = dynamic_cast<const Rectangle2D *>(other);
            if (rect) {
                return Mesh::collisionRectangles2D(my_parent_pos + this->getPosition(), this->getWidth(), this->getHeight(), other_parent_pos + other->getPosition(), rect->getWidth(), rect->getHeight());
            }
        } else if (other->getType() == "polygon2d" || other->getType() == "sphere2d") {
            const Polygon2D *poly = dynamic_cast<const Polygon2D *>(other);
            if (poly) {
                return Mesh::collisionRectangleCircle2D(my_parent_pos + this->getPosition(), this->getWidth(), this->getHeight(), other_parent_pos + other->getPosition(), poly->getRadius());
            }
        } else if (try_inverse) {
            return other->detectCollision(this, other_parent_pos, my_parent_pos, false);
        }
        return Mesh::detectCollision(other, my_parent_pos, other_parent_pos, try_inverse);
    }

};

#endif
