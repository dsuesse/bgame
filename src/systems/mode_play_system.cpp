#include "mode_play_system.hpp"
#include "../main/game_globals.hpp"
#include "gui_system.hpp"
#include "mouse_input_system.hpp"
#include <rltk.hpp>

using namespace rltk;
using namespace rltk::colors;

void mode_play_system::configure() {
    system_name = "Play mode";
}

void mode_play_system::update(const double duration_ms) {
    if (game_master_mode != PLAY) return;

    // Controls Help
	if (pause_mode == PAUSED) {
        add_gui_element(std::make_unique<map_static_text>( 34, 1, "SPACE", YELLOW));
        add_gui_element(std::make_unique<map_static_text>( 39, 1, "Unpause"));
	} else {
        add_gui_element(std::make_unique<map_static_text>( 34, 1, "SPACE", YELLOW));
        add_gui_element(std::make_unique<map_static_text>( 39, 1, "Pause"));
	}
    add_gui_element(std::make_unique<map_static_text>( 47, 1, "/", YELLOW));
    add_gui_element(std::make_unique<map_static_text>( 39, 1, "One Step"));

	// Since we're using an 8x8, it's just a matter of dividing by 8 to find the terminal-character
	// coordinates. There will be a helper function for this once we get into retained GUIs.
	const int world_x = std::min(clip_left + mouse::term1x, REGION_WIDTH);
	const int world_y = std::min(clip_top + mouse::term1y-2, REGION_HEIGHT);
	const int tile_idx = mapidx( world_x, world_y, camera_position->region_z );
	bool tooltip = false;
	
	if (mouse::term1x == last_mouse_x && mouse::term1y == last_mouse_y && world_y > 0) {
		mouse_dwell_time += duration_ms;
		if (mouse_dwell_time > 200.0 && tile_idx !=0 && current_region->revealed[tile_idx] && current_region->tile_type[tile_idx] != tile_type::OPEN_SPACE ) tooltip = true;
	} else {
		last_mouse_x = mouse::term1x;
		last_mouse_y = mouse::term1y;
		mouse_dwell_time = 0.0;
	}

	if (tooltip) {
		std::vector<std::string> lines;

		{
			std::stringstream ss;
			if (current_region->solid[tile_idx]) ss << "Solid-";
			if (current_region->tile_flags[tile_idx].test(CAN_GO_DOWN)) ss << "Down-";
			if (current_region->tile_flags[tile_idx].test(CAN_GO_UP)) ss << "Up-";
			if (current_region->tile_flags[tile_idx].test(CAN_GO_NORTH)) ss << "North-";
			if (current_region->tile_flags[tile_idx].test(CAN_GO_EAST)) ss << "East-";
			if (current_region->tile_flags[tile_idx].test(CAN_GO_WEST)) ss << "West-";
			if (current_region->tile_flags[tile_idx].test(CAN_GO_SOUTH)) ss << "South-";
			if (current_region->tile_flags[tile_idx].test(CAN_STAND_HERE)) ss << "Stand";
			lines.push_back(ss.str());
		}
		if (current_region->water_level[tile_idx]>0) lines.push_back(std::string("Water level: " + std::to_string(current_region->water_level[tile_idx])));

		{ // Base tile type
			std::stringstream ss;
			switch (current_region->tile_type[tile_idx]) {
				case tile_type::SEMI_MOLTEN_ROCK : ss << "Magma"; break;
				case tile_type::SOLID : ss << "Solid Rock (" << material_defs[current_region->tile_material[tile_idx]].name << ")"; break;
				case tile_type::OPEN_SPACE : ss << "Open Space"; break;
				case tile_type::WALL : ss << "Wall (" << material_defs[current_region->tile_material[tile_idx]].name << ")"; break;
				case tile_type::RAMP : ss << "Ramp (" << material_defs[current_region->tile_material[tile_idx]].name << ")"; break;
				case tile_type::STAIRS_UP : ss << "Up Stairs (" << material_defs[current_region->tile_material[tile_idx]].name << ")"; break;
				case tile_type::STAIRS_DOWN : ss << "Down Stairs (" << material_defs[current_region->tile_material[tile_idx]].name << ")"; break;
				case tile_type::STAIRS_UPDOWN : ss << "Spiral Stairs (" << material_defs[current_region->tile_material[tile_idx]].name << ")"; break;
				case tile_type::FLOOR : ss << "Floor (" << material_defs[current_region->tile_material[tile_idx]].name << ")"; break;
				case tile_type::TREE_TRUNK : ss << "Tree Trunk"; break;
				case tile_type::TREE_LEAF : ss << "Tree Foliage"; break;
				default : ss << "Unknown!";
			}
			lines.push_back(ss.str());
		}
		if (current_region->tile_type[tile_idx] == tile_type::FLOOR && !current_region->tile_flags[tile_idx].test(CONSTRUCTION)) {
			if (current_region->tile_vegetation_type[tile_idx] > 0) {
				lines.push_back(plant_defs[current_region->tile_vegetation_type[tile_idx]].name);
			}
		}

		// Named entities in the location
		each<name_t, position_t>([&lines,&world_x, &world_y] (entity_t &entity, name_t &name, position_t &pos) {
			if (pos.x == world_x && pos.y == world_y && pos.z == camera_position->region_z) {
				lines.push_back(name.first_name + std::string(" ") + name.last_name);
			}
		});
		// Items on the ground
		std::map<std::string, int> items;
		each<item_t, position_t>([&world_x, &world_y, &items] (entity_t &entity, item_t &item, position_t &pos) {
			if (pos.x == world_x && pos.y == world_y && pos.z == camera_position->region_z) {
				auto finder = items.find(item.item_name);
				if (finder == items.end()) {
					items[item.item_name] = 1;
				} else {
					++finder->second;
				}
			}
		});
		for (auto it=items.begin(); it!=items.end(); ++it) {
			std::string n = std::to_string(it->second);
			lines.push_back(n + std::string("x ") + it->first);
		}

		// Storage lockers and similar
		items.clear();
		each<construct_container_t, position_t>([&world_x, &world_y, &items] (entity_t &storage_entity, construct_container_t &container, position_t &pos) {
			if (pos.x == world_x && pos.y == world_y && pos.z == camera_position->region_z) {
				// It is a container and it is here - look inside!
				each<item_t, item_stored_t>([&items,&world_x, &world_y, &storage_entity] (entity_t &entity, item_t &item, item_stored_t &stored) {
					if (stored.stored_in == storage_entity.id) {
						auto finder = items.find(item.item_name);
						if (finder == items.end()) {
							items[item.item_name] = 1;
						} else {
							++finder->second;
						}
					}
				});
			}
		});
		for (auto it=items.begin(); it!=items.end(); ++it) {
			std::string n = std::to_string(it->second);
			lines.push_back(n + std::string("x ") + it->first);
		}

		// Buildings
		each<building_t, position_t>([&lines,&world_x, &world_y] (entity_t &building_entity, building_t &building, position_t &pos) {
			if (pos.x == world_x && pos.y == world_y && pos.z == camera_position->region_z) {
				// It's building and we can see it
				auto finder = building_defs.find(building.tag);
				std::string building_name = "Unknown Building";
				if (finder != building_defs.end()) {
					if (building.complete) {
						building_name = finder->second.name;
					} else {
						building_name = std::string("...") + finder->second.name;
					}
				}
				lines.push_back(building_name);
			}
		});

		int longest = 0;
		for (const std::string &s : lines) {
			if (s.size() > longest) longest = s.size();
		}

		// TODO - dynamic placement
		float revealed_pct = mouse_dwell_time - 200.0;
		revealed_pct /= game_config.tooltip_speed;
		if (revealed_pct > 1.0) revealed_pct = 1.0;
		if (!game_config.tooltip_fadein) revealed_pct = 1.0;

		if (revealed_pct < 1.0) {
			for (std::string &s : lines) {
				int n_garbled = s.size() - ((float)s.size() * (revealed_pct/2.0));
				for (int i=0; i<n_garbled; ++i) s[i] = static_cast<uint8_t>(rng.roll_dice(1,255));
			}
		}

		bool right_align = true;
		if (mouse::term1x > term(1)->term_width/2 ) right_align = false;
		int tt_y = mouse::term1y;
		if (tt_y+lines.size() > term(1)->term_height-1) tt_y -= lines.size()+1;

		if (right_align) {
			auto color = lerp(BLACK, LIGHT_GREEN, revealed_pct);
			int tt_x = mouse::term1x+2;
			term(1)->set_char(mouse::term1x+1, mouse::term1y, vchar{27, color, BLACK});
			term(1)->box(tt_x, tt_y, longest+1, lines.size()+1, color);
			++tt_y;
			for (const std::string &s : lines) {
				term(1)->print(tt_x+1, tt_y, s, color);
				++tt_y;
			}
		} else {
			auto color = lerp(BLACK, LIGHT_GREEN, revealed_pct);
			int tt_x = mouse::term1x-3-longest;
			term(1)->box(tt_x, tt_y, longest+1, lines.size()+1, color);
			++tt_y;
			for (const std::string &s : lines) {
				term(1)->print(tt_x+1, tt_y, s, color);
				++tt_y;
			}
			term(1)->set_char(mouse::term1x-1, mouse::term1y, vchar{26, color, BLACK});
		}
	}
}
