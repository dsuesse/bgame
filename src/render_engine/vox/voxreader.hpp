#pragma once

#include "voxel_model.hpp"
#include <string>

namespace vox {
	void load_vox(const std::string &filename, const std::string &tag);
	std::size_t model_index(const std::string tag);
	voxel_model * get_model(const std::size_t &index);
}