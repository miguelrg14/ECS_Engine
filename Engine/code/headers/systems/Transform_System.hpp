// Copyright (c) 2026 Miguel Rodriguez Gallego
// This code is released under the MIT License.

#pragma once

#include "System.hpp"

#include <cstdint>
#include <vector>

#include <glm/mat4x4.hpp>

namespace engine
{

    /// Computes Transform::world for every entity: world = parent's world * local.
    ///
    /// Parents are resolved on demand and each transform is computed once per
    /// frame (a stamp per entity index), so the order of the pool does not matter.
    /// A parent that is not a valid entity with a Transform is ignored: the child
    /// becomes a root. Parent chains longer than max_depth (or cycles) are cut there.
    class Transform_System final : public System
    {
        std::vector< std::uint32_t > stamps;      // frame in which each entity index was computed
        std::uint32_t                frame = 0;
        std::uint32_t                broken_chains = 0;

    public:

        static constexpr int max_depth = 64;

        explicit Transform_System (Registry & registry);

        void execute (float delta_time) override;

        /// Recomputes every world matrix now (also used by execute).
        void update ();

        /// Parent chains cut in the last update because of a cycle or excessive depth.
        std::uint32_t get_broken_chains () const { return broken_chains; }

    private:

        const glm::mat4 & compute (Entity entity, int depth);
    };

}
