#pragma once 

#include <iostream>
#include <time.h>
#include <vector>
#include <fstream>

#include "rkcommon/math/vec.h"
#include "rkcommon/math/box.h"

using namespace rkcommon::math;

struct Camera {
    vec3f pos;
    vec3f dir;
    vec3f up;

    Camera(const vec3f &pos, const vec3f &dir, const vec3f &up);
};

Camera::Camera(const vec3f &pos, const vec3f &dir, const vec3f &up)
    : pos(pos), dir(dir), up(up)
{
}

std::vector<vec3f> generate_fibonacci_sphere(const size_t n_points, const float radius)
{
    const float increment = M_PI * (3.f - std::sqrt(5.f));
    const float offset = 2.f / n_points;
    std::vector<vec3f> points;
    points.reserve(n_points);
    for (size_t i = 0; i < n_points; ++i) {
        const float y = ((i * offset) - 1.f) + offset / 2.f;
        const float r = std::sqrt(1.f - y * y);
        const float phi = i * increment;
        const float x = r * std::cos(phi);
        const float z = r * std::sin(phi);
        // std::cout << i << " " << x << " " << y << " " << z << "\n";
        points.emplace_back(x * radius, y * radius, z * radius);
    }
    return points;
}

std::vector<Camera> gen_cameras(const int num, const box3f &world_bounds){
    std::vector<Camera> cameras;
    std::srand((unsigned)time(NULL));
    int part = num / 100;
    for (int i = 0; i < part; i++){
        // float scale = ((float) rand()/RAND_MAX) * 2.5;
        float scale = 0.5;
        const float orbit_radius = length(world_bounds.size()) * scale;
        std::cout << "orbit radius: " << orbit_radius << std::endl;
        auto orbit_points = generate_fibonacci_sphere(num, orbit_radius);
        for (const auto &p : orbit_points) {
            cameras.emplace_back(p + world_bounds.center(), normalize(-p), vec3f(0, -1, 0));
        }
    }

    

    return cameras;
}

