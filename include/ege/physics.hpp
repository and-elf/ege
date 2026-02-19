#pragma once
#include <ege/physics/simple_physics.hpp>

// Backwards-compatible alias: keep `ege::PhysicsSystem` name for the
// simple header-only implementation (delegates to simple::SimplePhysics)
namespace ege {
namespace physics {
    using Simple = simple::SimplePhysics;
}
using PhysicsSystem = physics::Simple;

} // namespace ege
