// Copyright (c) 2026 Miguel Rodriguez Gallego
// This code is released under the MIT License.

#pragma once

#include "Mesh.hpp"

#include <memory>
#include <string>

namespace engine
{

    /// Loads a Wavefront .obj file into a single mesh: every object and group in
    /// the file is merged, polygons are triangulated, identical vertices shared,
    /// and smooth normals generated when the file has none.
    /// Throws std::runtime_error if the file cannot be read.
    std::shared_ptr< Mesh > load_obj (const std::string & path);

}
