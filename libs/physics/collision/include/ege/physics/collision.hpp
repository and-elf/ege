#pragma once

#include <cstddef>
#include <cmath>

namespace ege { namespace physics {

struct Vec2 { float x = 0.0f; float y = 0.0f; };

struct AABB { Vec2 center; float hx = 0.0f; float hy = 0.0f; };
struct Circle { Vec2 center; float r = 0.0f; };

inline bool aabb_vs_aabb(const AABB &a, const AABB &b) noexcept {
    float dx = std::fabs(a.center.x - b.center.x);
    float px = (a.hx + b.hx) - dx;
    if (px <= 0.0f) return false;
    float dy = std::fabs(a.center.y - b.center.y);
    float py = (a.hy + b.hy) - dy;
    return py > 0.0f;
}

inline bool circle_vs_circle(const Circle &a, const Circle &b) noexcept {
    float dx = a.center.x - b.center.x;
    float dy = a.center.y - b.center.y;
    float r = a.r + b.r;
    return (dx*dx + dy*dy) < (r*r);
}

} }
