#include "mesh.h"

namespace Engine {

    std::vector<std::valarray<double>> Mesh::implicitEquation (
        const std::valarray<double> ray_start,
        const std::valarray<double> ray_end
    ) {

        unsigned i = 0;
        const std::array<std::valarray<double>, 2> parametric = parametricEquation(ray_start, ray_end);
        std::vector<std::valarray<double>> result(std::max(ray_start.size(), ray_end.size()) - 1);

        for (auto &param : result) {
            param = { -parametric[1][i + 1], parametric[1][i], parametric[1][i] * parametric[0][i + 1] - parametric[1][i + 1] * parametric[0][i] };
            i++;
        }

        return result;
    }

    double Mesh::distanceRays (
        const std::valarray<double> &ray_1_start,
        const std::valarray<double> &ray_1_end,
        const std::valarray<double> &ray_2_start,
        const std::valarray<double> &ray_2_end,
        std::valarray<double> &ray_start,
        std::valarray<double> &ray_end
    ) {
        const std::valarray<double>
            ray_1_delta = ray_1_end - ray_1_start,
            ray_2_delta = ray_2_end - ray_2_start,
            rays_delta = ray_1_start - ray_2_start;

        const double
            ray_1_size2 = norm2(ray_1_delta),
            ray_2_size2 = norm2(ray_2_delta);

        double mua, mub;

        if (ray_1_size2 <= EPSILON) {

            if (ray_2_size2 <= EPSILON) {
                mua = 0.0, mub = 0.0;
            } else {
                mua = 0.0;
                mub = clamp(dot(ray_2_delta, rays_delta) / ray_2_size2, 0.0, 1.0);
            }
        } else {
            const double c = dot(ray_1_delta, rays_delta);

            if (ray_2_size2 <= EPSILON) {
                mub = 0.0;
                mua = clamp(-c / ray_1_size2, 0.0, 1.0);
            } else {
                const double
                    b = dot(ray_1_delta, ray_2_delta),
                    denom = ray_1_size2 * ray_2_size2 - b * b,
                    f = dot(ray_2_delta, rays_delta);

                if (denom != 0.0) {
                    mua = clamp((b * f - c * ray_2_size2) / denom, 0.0, 1.0);
                } else {
                    mua = 0.0;
                }

                const double numer = b * mua + f;

                if (numer <= 0.0) {
                    mub = 0.0;
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

        return distance(ray_start, ray_end);
    }

    bool Mesh::collisionPointTriangle2D (
        const std::valarray<double> &point,
        const std::valarray<double> &tri_point_1,
        const std::valarray<double> &tri_point_2,
        const std::valarray<double> &tri_point_3
    ) {
        double sum = 0.0;

        for (const auto &edge : edgesTriangle2D(tri_point_1, tri_point_2, tri_point_3)) {
            sum += areaTriangle2D(edge[0], point, edge[1]);
        }

        return sum <= (areaTriangle2D(tri_point_1, tri_point_2, tri_point_3) + EPSILON);
    }

    bool Mesh::collisionPointRectangle2D (
        const std::valarray<double> &point,
        const std::valarray<double> &rect_top_left,
        const std::valarray<double> &rect_bottom_left,
        const std::valarray<double> &rect_bottom_right,
        const std::valarray<double> &rect_top_right
    ) {
        double sum = 0.0;

        for (const auto &edge : edgesRectangle2D(rect_top_left, rect_bottom_left, rect_bottom_right, rect_top_right)) {
            sum += areaTriangle2D(edge[0], point, edge[1]);
        }

        return sum <= (areaRectangle2D(rect_top_left, rect_bottom_left, rect_bottom_right) + EPSILON);
    }

    bool Mesh::collisionRectangles2D (
        const std::valarray<double> &rect_1_top_left,
        const std::valarray<double> &rect_1_bottom_left,
        const std::valarray<double> &rect_1_bottom_right,
        const std::valarray<double> &rect_1_top_right,
        const std::valarray<double> &rect_2_top_left,
        const std::valarray<double> &rect_2_bottom_left,
        const std::valarray<double> &rect_2_bottom_right,
        const std::valarray<double> &rect_2_top_right,
        std::valarray<double> &near_point
    ) {
        const std::array<std::array<std::valarray<double>, 2>, 4>
            rect_1_edges = edgesRectangle2D(rect_1_top_left, rect_1_bottom_left, rect_1_bottom_right, rect_1_top_right),
            rect_2_edges = edgesRectangle2D(rect_2_top_left, rect_2_bottom_left, rect_2_bottom_right, rect_2_top_right);

        std::valarray<double> ray_end;

        for (const auto &rect_1_edge : rect_1_edges) {
            for (const auto &rect_2_edge : rect_2_edges) {
                if (distanceRays(rect_1_edge[0], rect_1_edge[1], rect_2_edge[0], rect_2_edge[1], near_point, ray_end) <= EPSILON) {
                    return true;
                }
            }
        }

        for (const auto &rect_2_vertex : { rect_2_top_left, rect_2_bottom_left, rect_2_bottom_right, rect_2_top_right }) {
            if (collisionPointRectangle2D(rect_1_top_left, rect_1_bottom_left, rect_1_bottom_right, rect_1_top_right, rect_2_vertex)) {
                near_point = rect_2_vertex;
                return true;
            }
        }

        return false;
    }

};
