// Copyright (c) 2026 Miguel Rodriguez Gallego
// This code is released under the MIT License.

#pragma once

#include <cstdint>

namespace engine
{

    /// An entity is only an identifier: it has no data and no behaviour of its own.
    ///
    /// The 32 bits hold two fields:
    /// - index   (low 24 bits): slot in the registry, reused after the entity is destroyed.
    /// - version (high 8 bits): incremented every time the slot is reused, so a handle
    ///   kept from a destroyed entity is detected as invalid instead of silently
    ///   pointing at the new one.
    using Entity = std::uint32_t;

    namespace entity
    {
        inline constexpr std::uint32_t index_bits    = 24;
        inline constexpr std::uint32_t index_mask    = (1u << index_bits) - 1u;     // 0x00FFFFFF
        inline constexpr std::uint32_t version_mask  = 0xFFu;

        /// Largest number of entities alive at the same time (16,777,215).
        inline constexpr std::uint32_t max_entities  = index_mask;

        constexpr std::uint32_t index_of   (Entity entity) { return entity & index_mask; }
        constexpr std::uint32_t version_of (Entity entity) { return (entity >> index_bits) & version_mask; }

        constexpr Entity make (std::uint32_t index, std::uint32_t version)
        {
            return (index & index_mask) | ((version & version_mask) << index_bits);
        }
    }

    /// Never returned by Registry::create; use it for "no entity".
    inline constexpr Entity null_entity = 0xFFFFFFFFu;

}
