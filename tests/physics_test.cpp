#include <gtest/gtest.h>

#include <ege/physics.hpp>

using namespace ege;

static constexpr float EPS = 1e-3f;

TEST(PhysicsTest, AABBResolutionSeparates)
{
    PhysicsSystem ps;
    Body a;
    a.pos = {0.0f, 0.0f};
    a.vel = {0.0f, 0.0f};
    a.inv_mass = 1.0f;
    a.collider.type = ColliderType::AABB;
    a.collider.data.aabb.hx = 1.0f;
    a.collider.data.aabb.hy = 1.0f;

    Body b = a;
    b.pos = {0.5f, 0.0f}; // overlapping in x

    auto ida = ps.add_body(a);
    auto idb = ps.add_body(b);

    ps.step(1.0f);

    const Body &ra = ps.body(ida);
    const Body &rb = ps.body(idb);
    float dx = std::fabs(rb.pos.x - ra.pos.x);
    float target = a.collider.data.aabb.hx + b.collider.data.aabb.hx;
    EXPECT_GE(dx + EPS, target);
}

TEST(PhysicsTest, CircleResolutionSeparates)
{
    PhysicsSystem ps;
    Body a;
    a.pos = {0.0f, 0.0f};
    a.vel = {0.0f, 0.0f};
    a.inv_mass = 1.0f;
    a.collider.type = ColliderType::Circle;
    a.collider.data.circle.r = 1.0f;

    Body b = a;
    b.pos = {0.5f, 0.0f};

    auto ida = ps.add_body(a);
    auto idb = ps.add_body(b);

    ps.step(1.0f);

    const Body &ra = ps.body(ida);
    const Body &rb = ps.body(idb);
    float dx = std::sqrt((rb.pos.x - ra.pos.x)*(rb.pos.x - ra.pos.x) + (rb.pos.y - ra.pos.y)*(rb.pos.y - ra.pos.y));
    float target = a.collider.data.circle.r + b.collider.data.circle.r;
    EXPECT_GE(dx + EPS, target);
}

TEST(PhysicsTest, StaticBodyDoesNotMove)
{
    PhysicsSystem ps;
    Body a;
    a.pos = {0.0f, 0.0f};
    a.vel = {0.0f, 0.0f};
    a.inv_mass = 0.0f; // static
    a.collider.type = ColliderType::AABB;
    a.collider.data.aabb.hx = 1.0f;
    a.collider.data.aabb.hy = 1.0f;

    Body b = a;
    b.pos = {0.5f, 0.0f};
    b.inv_mass = 1.0f;

    auto ida = ps.add_body(a);
    auto idb = ps.add_body(b);

    ps.step(1.0f);

    const Body &ra = ps.body(ida);
    const Body &rb = ps.body(idb);
    // static body should not change
    EXPECT_NEAR(ra.pos.x, 0.0f, EPS);
    EXPECT_NEAR(ra.pos.y, 0.0f, EPS);
    // other body should be moved away from static
    float dx = std::fabs(rb.pos.x - ra.pos.x);
    float target = a.collider.data.aabb.hx + b.collider.data.aabb.hx;
    EXPECT_GE(dx + EPS, target);
}
