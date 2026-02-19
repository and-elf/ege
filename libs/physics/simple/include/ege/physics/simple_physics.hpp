#pragma once

#include <vector>
#include <cstdint>
#include <cmath>
#include <ege/physics/collision.hpp>

namespace ege { namespace physics {

using BodyId = uint32_t;

struct Body {
    Vec2 pos{};
    Vec2 vel{};
    float inv_mass = 1.0f; // zero = static
    // simple AABB for now
    float hx = 0.5f, hy = 0.5f;
    float radius = 0.0f; // if >0, treat as circle
};

class SimplePhysics {
public:
    SimplePhysics() = default;

    BodyId add_body(const Body &b) {
        bodies_.push_back(b);
        return static_cast<BodyId>(bodies_.size()-1);
    }
    Body &body(BodyId id) { return bodies_.at(id); }
    const Body &body(BodyId id) const { return bodies_.at(id); }

    void step(float dt) {
        if (dt <= 0.0f) return;
        for (auto &b : bodies_) {
            if (b.inv_mass == 0.0f) continue;
            b.pos.x += b.vel.x * dt;
            b.pos.y += b.vel.y * dt;
        }
        // naive n^2
        const std::size_t n = bodies_.size();
        for (std::size_t i = 0; i < n; ++i) {
            for (std::size_t j = i+1; j < n; ++j) {
                resolve_pair(bodies_[i], bodies_[j]);
            }
        }
    }

private:
    std::vector<Body> bodies_;

    static void resolve_pair(Body &a, Body &b) {
        if (a.radius > 0.0f && b.radius > 0.0f) {
            physics::Circle A{a.pos, a.radius};
            physics::Circle B{b.pos, b.radius};
            if (!physics::circle_vs_circle(A,B)) return;
            // simple separation
            float dx = b.pos.x - a.pos.x;
            float dy = b.pos.y - a.pos.y;
            float dist = std::sqrt(dx*dx + dy*dy);
            if (dist == 0.0f) dist = 1e-4f;
            float pen = (a.radius + b.radius) - dist;
            float nx = dx / dist, ny = dy / dist;
            float corr = pen * 0.5f;
            if (a.inv_mass > 0.0f) { a.pos.x -= nx*corr; a.pos.y -= ny*corr; }
            if (b.inv_mass > 0.0f) { b.pos.x += nx*corr; b.pos.y += ny*corr; }
        } else {
            physics::AABB A{a.pos, a.hx, a.hy};
            physics::AABB B{b.pos, b.hx, b.hy};
            if (!physics::aabb_vs_aabb(A,B)) return;
            float dx = b.pos.x - a.pos.x;
            float px = (a.hx + b.hx) - std::fabs(dx);
            float dy = b.pos.y - a.pos.y;
            float py = (a.hy + b.hy) - std::fabs(dy);
            if (px < py) {
                float sx = (dx < 0.0f) ? -1.0f : 1.0f;
                float corr = px * 0.5f;
                if (a.inv_mass > 0.0f) a.pos.x -= sx*corr;
                if (b.inv_mass > 0.0f) b.pos.x += sx*corr;
            } else {
                float sy = (dy < 0.0f) ? -1.0f : 1.0f;
                float corr = py * 0.5f;
                if (a.inv_mass > 0.0f) a.pos.y -= sy*corr;
                if (b.inv_mass > 0.0f) b.pos.y += sy*corr;
            }
        }
    }
};

} }
