#include <gtest/gtest.h>

#include <ege/physics.hpp>
#include <ege/physics/collision.hpp>

using namespace ege;
using namespace ege::physics;

static constexpr float EPS = 1e-3f;

TEST(PhysicsTest, AABBResolutionSeparates)
{
    PhysicsSystem ps;
    Body a;
    a.pos = {0.0f, 0.0f};
    a.vel = {0.0f, 0.0f};
    a.inv_mass = 1.0f;
    a.hx = 1.0f;
    a.hy = 1.0f;

    Body b = a;
    b.pos = {0.5f, 0.0f}; // overlapping in x

    auto ida = ps.add_body(a);
    auto idb = ps.add_body(b);

    ps.step(1.0f);

    const Body &ra = ps.body(ida);
    const Body &rb = ps.body(idb);
    float dx = std::fabs(rb.pos.x - ra.pos.x);
    float target = a.hx + b.hx;
    EXPECT_GE(dx + EPS, target);
}

TEST(PhysicsTest, CircleResolutionSeparates)
{
    PhysicsSystem ps;
    Body a;
    a.pos = {0.0f, 0.0f};
    a.vel = {0.0f, 0.0f};
    a.inv_mass = 1.0f;
    a.radius = 1.0f;

    Body b = a;
    b.pos = {0.5f, 0.0f};

    auto ida = ps.add_body(a);
    auto idb = ps.add_body(b);

    ps.step(1.0f);

    const Body &ra = ps.body(ida);
    const Body &rb = ps.body(idb);
    float dx = std::sqrt((rb.pos.x - ra.pos.x)*(rb.pos.x - ra.pos.x) + (rb.pos.y - ra.pos.y)*(rb.pos.y - ra.pos.y));
    float target = a.radius + b.radius;
    EXPECT_GE(dx + EPS, target);
}

TEST(PhysicsTest, StaticBodyDoesNotMove)
{
    PhysicsSystem ps;
    Body a;
    a.pos = {0.0f, 0.0f};
    a.vel = {0.0f, 0.0f};
    a.inv_mass = 0.0f; // static
    a.hx = 1.0f;
    a.hy = 1.0f;

    Body b = a;
    b.pos = {0.5f, 0.0f};
    b.inv_mass = 1.0f;

    auto ida = ps.add_body(a);
    auto idb = ps.add_body(b);

    float initial_dx = std::fabs(b.pos.x - a.pos.x);
    ps.step(1.0f);

    const Body &ra = ps.body(ida);
    const Body &rb = ps.body(idb);
    // static body should not change
    EXPECT_NEAR(ra.pos.x, 0.0f, EPS);
    EXPECT_NEAR(ra.pos.y, 0.0f, EPS);
    // other body should be moved away from static (at least from initial)
    float dx = std::fabs(rb.pos.x - ra.pos.x);
    EXPECT_GT(dx, initial_dx);
}
