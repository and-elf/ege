#pragma once
#include <ege/physics/simple_physics.hpp>

// Backwards-compatible alias: keep `ege::PhysicsSystem` name for the
// simple header-only implementation. The simple physics implementation
// lives in `ege::physics::SimplePhysics` so create a short alias.
namespace ege {
namespace physics {
    using Simple = SimplePhysics;
}
using PhysicsSystem = physics::Simple;

} // namespace ege
