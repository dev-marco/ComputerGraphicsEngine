#include "mesh.h"

namespace Engine {

    float_max_t Mesh::distanceRayRay (
        const Vec<3> &ray_1_start,
        const Vec<3> &ray_1_end,
        const Vec<3> &ray_2_start,
        const Vec<3> &ray_2_end,
        Vec<3> &ray_start,
        Vec<3> &ray_end
    ) {
        const Vec<3>
            ray_1_delta = ray_1_end - ray_1_start,
            ray_2_delta = ray_2_end - ray_2_start,
            rays_delta = ray_1_start - ray_2_start;

        const float_max_t
            ray_1_size2 = ray_1_delta.length2(),
            ray_2_size2 = ray_2_delta.length2();

        float_max_t mua = 0.0, mub = 0.0;

        if (ray_1_size2 <= EPSILON) {
            if (ray_2_size2 > EPSILON) {
                mub = clamp(ray_2_delta.dot(rays_delta) / ray_2_size2, 0.0, 1.0);
            }
        } else {
            const float_max_t c = ray_1_delta.dot(rays_delta);

            if (ray_2_size2 <= EPSILON) {
                mua = clamp(-c / ray_1_size2, 0.0, 1.0);
            } else {
                const float_max_t
                    b = ray_1_delta.dot(ray_2_delta),
                    denom = ray_1_size2 * ray_2_size2 - b * b,
                    f = ray_2_delta.dot(rays_delta);

                if (denom != 0.0) {
                    mua = clamp((b * f - c * ray_2_size2) / denom, 0.0, 1.0);
                }

                const float_max_t numer = b * mua + f;

                if (numer <= 0.0) {
                    mua = clamp(-c / ray_1_size2, 0.0, 1.0);
                } else if (numer >= ray_2_size2) {
                    mub = 1.0;
                    mua = clamp((b - c) / ray_1_size2, 0.0, 1.0);
                } else {
                    mub = numer / ray_2_size2;
                }
            }
        }

        ray_start = ray_1_start + ray_1_delta * mua;
        ray_end = ray_2_start + ray_2_delta * mub;

        return ray_start.distance(ray_end);
    }

    bool Mesh::intersectionRectangleRectangle (
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
    ) {

        if (rect_1_orientation == rect_2_orientation) {

            if (rect_1_orientation.isIdentity()) {

                return
                    rect_1_top_left[0] < rect_2_bottom_right[0] &&
                    rect_1_bottom_right[0] > rect_2_top_left[0] &&
                    rect_1_top_left[1] < rect_2_bottom_right[1] &&
                    rect_1_bottom_right[1] > rect_2_top_left[1];

            } else {

                const Quaternion inv_orientation = -rect_1_orientation;

                const Vec<3>
                    rot_rect_1_top_left = inv_orientation.rotated(rect_1_top_left),
                    rot_rect_1_bottom_right = inv_orientation.rotated(rect_1_bottom_right),
                    rot_rect_2_top_left = inv_orientation.rotated(rect_2_top_left),
                    rot_rect_2_bottom_right = inv_orientation.rotated(rect_2_bottom_right);

                return
                    rot_rect_1_top_left[0] < rot_rect_2_bottom_right[0] &&
                    rot_rect_1_bottom_right[0] > rot_rect_2_top_left[0] &&
                    rot_rect_1_top_left[1] < rot_rect_2_bottom_right[1] &&
                    rot_rect_1_bottom_right[1] > rot_rect_2_top_left[1];

            }

        } else {
            const std::array<std::array<Vec<3>, 2>, 4>
                rect_1_edges = edgesRectangle(rect_1_top_left, rect_1_bottom_left, rect_1_bottom_right, rect_1_top_right),
                rect_2_edges = edgesRectangle(rect_2_top_left, rect_2_bottom_left, rect_2_bottom_right, rect_2_top_right);

            Vec<3> ray_end;

            for (unsigned i = 0; i < rect_1_edges.size(); ++i) {
                for (unsigned j = 0; j < rect_2_edges.size(); ++j) {
                    if (distanceRayRay(rect_1_edges[i][0], rect_1_edges[i][1], rect_2_edges[j][0], rect_2_edges[j][1], near_point, ray_end) <= EPSILON) {
                        return true;
                    }
                }
            }

            for (const auto &rect_2_vertex : { rect_2_top_left, rect_2_bottom_left, rect_2_bottom_right, rect_2_top_right }) {
                if (intersectionPointRectangle2D(rect_1_top_left, rect_1_bottom_left, rect_1_bottom_right, rect_1_top_right, rect_2_vertex)) {
                    near_point = rect_2_vertex;
                    return true;
                }
            }
        }

        return false;
    }

    void Mesh::draw (const bool only_border) const {

        Draw::push();

        Draw::translate(this->getPosition());
        Draw::rotate(this->getOrientation());

        this->_draw(only_border);

        if (!this->children.empty()) {
            for (const auto &mesh : this->children) {
                mesh->draw(only_border);
            }
        }

        Draw::pop();
    }

};
