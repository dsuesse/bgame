#include "panel_render_system.hpp"
#include "../game_globals.hpp"
#include "../raws/raws.hpp"
#include "../components/components.hpp"
#include "../messages/messages.hpp"
#include "mining_system.hpp"
#include "inventory_system.hpp"
#include <sstream>
#include <iomanip>

using namespace rltk;
using namespace rltk::colors;

std::vector<available_building_t> available_buildings;

const color_t GREEN_BG{0,32,0};

void panel_render_system::update(const double duration_ms) {
	mouse_damper += duration_ms;

	term(3)->clear(vchar{' ', WHITE, GREEN_BG});
	term(3)->box(DARKEST_GREEN);

	render_mode_select();

	if (game_master_mode == PLAY) {
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
			game_master_mode = DESIGN;
			pause_mode = PAUSED;
			emit(map_dirty_message{});
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::U)) {
			game_master_mode = UNITS;
			pause_mode = PAUSED;
			emit(map_dirty_message{});
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) {
			game_master_mode = WORKFLOW;
			pause_mode = PAUSED;
			emit(map_dirty_message{});
		}
	} else if (game_master_mode == DESIGN) {
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Escape)) {
			game_master_mode = PLAY;
			emit(map_dirty_message{});
			emit(recalculate_mining_message{});
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
			game_design_mode = DIGGING;
			emit(map_dirty_message{});
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::B)) {
			game_design_mode = BUILDING;
			available_buildings = get_available_buildings();
			emit(map_dirty_message{});
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::T)) {
			game_design_mode = CHOPPING;
			emit(map_dirty_message{});
		}
	} else if (game_master_mode == UNITS) {
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Escape)) {
			game_master_mode = PLAY;
			emit(map_dirty_message{});
			emit(recalculate_mining_message{});
		}
	} else if (game_master_mode == SETTLER) {
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Escape)) {
			game_master_mode = PLAY;
			emit(map_dirty_message{});
			emit(recalculate_mining_message{});
		}
	} else if (game_master_mode == WORKFLOW) {
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Escape)) {
			game_master_mode = PLAY;
			emit(map_dirty_message{});
			emit(recalculate_mining_message{});
		}
	}
}

void panel_render_system::configure() {
	system_name = "Panel Render";
	subscribe<refresh_available_buildings_message>([this] (refresh_available_buildings_message &msg) {
		available_buildings = get_available_buildings();
	});
}

void panel_render_system::render_mode_select() {
	// Mode switch controls
	if (game_master_mode == PLAY) {
		term(2)->print(1,1,"PLAY", WHITE, DARKEST_GREEN);
		render_play_mode();
	} else {
		term(2)->print(1,1,"(ESC) PLAY", GREEN, GREEN_BG);
	}

	if (game_master_mode == DESIGN) {
		term(2)->print(13,1,"DESIGN", WHITE, DARKEST_GREEN);
		render_design_mode();
	} else {
		term(2)->print(10,1,"(D)ESIGN", GREEN, GREEN_BG);
	}

	if (game_master_mode == UNITS) {
		term(2)->print(23,1,"UNITS", WHITE, DARKEST_GREEN);
		render_units_mode();
	} else {
		term(2)->print(20,1,"(U)NITS", GREEN, GREEN_BG);
	}

	if (game_master_mode == WORKFLOW) {
		term(2)->print(32,1,"WORK", WHITE, DARKEST_GREEN);
		render_work_mode();
	} else {
		term(2)->print(29,1,"(W)ORK", GREEN, GREEN_BG);
	}

	if (game_master_mode == SETTLER) {
		render_settler_mode();
	}
}

void panel_render_system::render_play_mode() {
	// Controls Help
	if (pause_mode == PAUSED) {
		term(3)->print(1,3,"SPACE : Unpause", GREEN, GREEN_BG);
	} else {
		term(3)->print(1,3,"SPACE : Pause", GREEN, GREEN_BG);
	}
	term(3)->print(1,4,".     : One Step", GREEN, GREEN_BG);

	// Mouse tips
	int mouse_x, mouse_y;
	std::tie(mouse_x, mouse_y) = get_mouse_position();

	// Since we're using an 8x8, it's just a matter of dividing by 8 to find the terminal-character
	// coordinates. There will be a helper function for this once we get into retained GUIs.
	const int terminal_x = mouse_x / 8;
	const int terminal_y = mouse_y / 8;

	/*
	if (terminal_x >= 0 && terminal_x < term(1)->term_width && terminal_y >= 0 && terminal_y < term(1)->term_height) {
		const int world_x = std::min(clip_left + terminal_x, REGION_WIDTH);
		const int world_y = std::min(clip_top + terminal_y-2, REGION_HEIGHT);
		const int idx = mapidx(world_x, world_y, camera_position->region_z);

		{
			const int base_tile_type = current_region->tiles[idx].base_type;
			std::stringstream ss;
			auto finder = tile_types.find(base_tile_type);
			if (finder != tile_types.end()) {
				ss << finder->second.name;
				term(3)->print(1, term(3)->term_height - 2, ss.str(), GREEN, GREEN_BG);
			}
		}
		{
			const int base_tile_content = current_region->tiles[idx].contents;
			std::stringstream ss;
			auto finder = tile_contents.find(base_tile_content);
			if (finder != tile_contents.end()) {
				ss << finder->second.name;
				term(3)->print(1, term(3)->term_height - 3, ss.str(), GREEN, GREEN_BG);
			}
		}
		{
			std::stringstream ss;
			if (current_region->solid[idx]) ss << "S";
			if (current_region->tiles[idx].flags.test(tile_flags::TREE)) ss << "T";
			if (current_region->tiles[idx].flags.test(tile_flags::CONSTRUCTION)) ss << "C";
			if (current_region->tiles[idx].flags.test(tile_flags::CAN_GO_NORTH)) ss << "N";
			if (current_region->tiles[idx].flags.test(tile_flags::CAN_GO_SOUTH)) ss << "S";
			if (current_region->tiles[idx].flags.test(tile_flags::CAN_GO_EAST)) ss << "E";
			if (current_region->tiles[idx].flags.test(tile_flags::CAN_GO_WEST)) ss << "W";
			if (current_region->tiles[idx].flags.test(tile_flags::CAN_GO_UP)) ss << "U";
			if (current_region->tiles[idx].flags.test(tile_flags::CAN_GO_DOWN)) ss << "D";
			ss << current_region->tiles[idx].tree_id;
			term(3)->print(1, term(3)->term_height - 4, ss.str(), GREEN, GREEN_BG);
		}
		int count = 0;
		// Named entities in the location
		each<name_t, position_t>([&count, &world_x, &world_y] (entity_t &entity, name_t &name, position_t &pos) {
			if (pos.x == world_x && pos.y == world_y && pos.z == camera_position->region_z) {
				term(3)->print(1, term(3)->term_height - 5 - count, name.first_name + std::string(" ") + name.last_name, GREEN, GREEN_BG);
				++count;
			}
		});
		// Items on the ground
		each<item_t, position_t>([&count, &world_x, &world_y] (entity_t &entity, item_t &item, position_t &pos) {
			if (pos.x == world_x && pos.y == world_y && pos.z == camera_position->region_z) {
				term(3)->print(1, term(3)->term_height - 5 - count, item.item_name, GREEN, GREEN_BG);
				++count;
			}
		});
		// Storage lockers and similar
		each<construct_container_t, position_t>([&count, &world_x, &world_y] (entity_t &storage_entity, construct_container_t &container, position_t &pos) {
			if (pos.x == world_x && pos.y == world_y && pos.z == camera_position->region_z) {
				// It is a container and it is here - look inside!
				each<item_t, item_stored_t>([&count, &world_x, &world_y, &storage_entity] (entity_t &entity, item_t &item, item_stored_t &stored) {
					if (stored.stored_in == storage_entity.id) {
						term(3)->print(1, term(3)->term_height - 5 - count, item.item_name, GREEN, GREEN_BG);
						++count;
					}
				});
			}
		});
		// Buildings
		each<building_t, position_t>([&count, &world_x, &world_y] (entity_t &building_entity, building_t &building, position_t &pos) {
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
				term(3)->print(1, term(3)->term_height-5-count, building_name, GREEN, GREEN_BG);
				++count;
			}
		});
	}*/
}

inline bool is_mining_designation_valid(const int &x, const int &y, const int &z, const game_mining_mode_t &mode) {
	return true;
}

void panel_render_system::render_design_mode() {
	if (game_design_mode == DIGGING) {
		term(3)->print(1,3, "Digging", WHITE, DARKEST_GREEN);

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) game_mining_mode = DIG;
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::C)) game_mining_mode = CHANNEL;
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::R)) game_mining_mode = RAMP;
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::U)) game_mining_mode = UP;
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::J)) game_mining_mode = DOWN;
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::I)) game_mining_mode = UPDOWN;
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::X)) game_mining_mode = DELETE;

		if (game_mining_mode == DIG) { term(3)->print(1,8, "(d) Dig", WHITE, DARKEST_GREEN); } else { term(3)->print(1,8, "(d) Dig", GREEN, GREEN_BG); }
		if (game_mining_mode == CHANNEL) { term(3)->print(1,9, "(c) Channel", WHITE, DARKEST_GREEN); } else { term(3)->print(1,9, "(c) Channel", GREEN, GREEN_BG); }
		if (game_mining_mode == RAMP) { term(3)->print(1,10, "(r) Ramp", WHITE, DARKEST_GREEN); } else { term(3)->print(1,10, "(r) Ramp", GREEN, GREEN_BG); }
		if (game_mining_mode == UP) { term(3)->print(1,11, "(u) Up Stairs", WHITE, DARKEST_GREEN); } else { term(3)->print(1,11, "(u) Up Stairs", GREEN, GREEN_BG); }
		if (game_mining_mode == DOWN) { term(3)->print(1,12, "(j) Down Stairs", WHITE, DARKEST_GREEN); } else { term(3)->print(1,12, "(j) Down Stairs", GREEN, GREEN_BG); }
		if (game_mining_mode == UPDOWN) { term(3)->print(1,13, "(i) Up/Down Stairs", WHITE, DARKEST_GREEN); } else { term(3)->print(1,13, "(i) Up/Down Stairs", GREEN, GREEN_BG); }
		if (game_mining_mode == DELETE) { term(3)->print(1,14, "(x) Clear", WHITE, DARKEST_GREEN); } else { term(3)->print(1,14, "(x) Clear", GREEN, GREEN_BG); }

		int mouse_x, mouse_y;
		std::tie(mouse_x, mouse_y) = get_mouse_position();
		const int terminal_x = mouse_x / 8;
		const int terminal_y = mouse_y / 8;

		if (terminal_x >= 0 && terminal_x < term(1)->term_width && terminal_y >= 0 && terminal_y < term(1)->term_height) {
			if (get_mouse_button_state(rltk::button::LEFT)) {
				const int world_x = std::min(clip_left + terminal_x, REGION_WIDTH);
				const int world_y = std::min(clip_top + terminal_y-2, REGION_HEIGHT);
				const int idx = mapidx(world_x, world_y, camera_position->region_z);
				if (is_mining_designation_valid(world_x, world_y, camera_position->region_z, game_mining_mode)) {
					switch (game_mining_mode) {
						case DIG : designations->mining[idx] = 1; break;
						case CHANNEL : designations->mining[idx] = 2; break;
						case RAMP : designations->mining[idx] = 3; break;
						case UP : designations->mining[idx] = 4; break;
						case DOWN : designations->mining[idx] = 5; break;
						case UPDOWN : designations->mining[idx] = 6; break;
						case DELETE : designations->mining.erase(idx); break;
					}
					emit(map_dirty_message{});
				}
			}
		}

	} else {
		term(3)->print(1,3, "(D)igging", GREEN, GREEN_BG);
	}

	if (game_design_mode == BUILDING) {
		term(3)->print(1,4, "Building", WHITE, DARKEST_GREEN);

		int y=8;
		for (const available_building_t &building : available_buildings) {
			if (build_mode_building && build_mode_building.get().tag == building.tag) {
				term(3)->print(1, y, building.name, WHITE, DARKEST_GREEN);
			} else {
				term(3)->print(1, y, building.name, GREEN, GREEN_BG);
			}
			++y;
		}

		if (get_mouse_button_state(rltk::button::LEFT)) {
			int mouse_x, mouse_y;
			std::tie(mouse_x, mouse_y) = get_mouse_position();

			if (mouse_x > layer(3)->x && mouse_y > layer(3)->y && mouse_x < layer(3)->x+layer(3)->w && mouse_y < layer(3)->y + layer(3)->h ) {
				mouse_x -= layer(3)->x;
				mouse_y -= layer(3)->y;
				int terminal_x = mouse_x / 8;
				int terminal_y = mouse_y / 16;
				
				if (terminal_y > 7 && terminal_y < 8+available_buildings.size()) {
					const int selected_building = terminal_y - 8;
					build_mode_building = available_buildings[selected_building];
				}
			}
		}
	} else {
		term(3)->print(1,4, "(B)uilding", GREEN, GREEN_BG);
	}

	if (game_design_mode == CHOPPING) {
		term(3)->print(1,5, "Tree Chopping", WHITE, DARKEST_GREEN);

		int mouse_x, mouse_y;
		std::tie(mouse_x, mouse_y) = get_mouse_position();
		const int terminal_x = mouse_x / 8;
		const int terminal_y = mouse_y / 8;

		if (terminal_x >= 0 && terminal_x < term(1)->term_width && terminal_y >= 0 && terminal_y < term(1)->term_height) {
			const int world_x = std::min(clip_left + terminal_x, REGION_WIDTH);
			const int world_y = std::min(clip_top + terminal_y-2, REGION_HEIGHT);

			const int idx = mapidx(world_x, world_y, camera_position->region_z);
			const int tree_id = current_region->tree_id[idx];

			if (get_mouse_button_state(rltk::button::LEFT) && tree_id > 0) {
				// Naieve search for the base of the tree; this could be optimized a LOT
				int lowest_z = camera_position->region_z;
				const int stop_z = lowest_z-10;

				position_t tree_pos{world_x, world_y, lowest_z};
				while (lowest_z > stop_z) {
					for (int y=-10; y<10; ++y) {
						for (int x=-10; x<10; ++x) {
							const int tree_idx = mapidx(world_x + x, world_y + y, lowest_z);
							if (current_region->tree_id[tree_idx] == tree_id) {
								tree_pos.x = world_x+x;
								tree_pos.y = world_y+y;
								tree_pos.z = lowest_z;
							}
						}
					}
					--lowest_z;
				}

				designations->chopping[tree_id] = tree_pos;
				emit(map_dirty_message{});
			} else if (get_mouse_button_state(rltk::button::RIGHT) && tree_id > 0) {
				designations->chopping.erase(tree_id);
				emit(map_dirty_message{});
			}
			
		}
	} else {
		term(3)->print(1,5, "(T)ree Chopping", GREEN, GREEN_BG);
	}
}

std::string max_width_str(const std::string original, const int width) {
	if (original.size() <= width) return original;
	return original.substr(0,width);
}

void panel_render_system::render_units_mode() {
	int y = 5;
	term(1)->box(3, 2, 70, 40, WHITE, BLACK, true);
	for (int i=3; i<42; ++i) term(1)->print(4, i, "                                                                     ");

	term(1)->print(5,4,"Settler Name        Profession     Current Status", YELLOW, BLACK);

	int mouse_x, mouse_y;
	std::tie(mouse_x, mouse_y) = get_mouse_position();
	int terminal_x = mouse_x/8;
	int terminal_y = (mouse_y/8);

	each<settler_ai_t, name_t, game_stats_t>([this, &y, &terminal_x, &terminal_y] (entity_t &e, settler_ai_t &ai, name_t &name, game_stats_t &stats) {
		color_t background = BLACK;
		
		if (terminal_y == y && terminal_x > 4 && terminal_x < 70) {
			background = BLUE;

			if (get_mouse_button_state(rltk::button::LEFT)) {
				selected_settler = e.id;
				game_master_mode = SETTLER;
			}
		}

		term(1)->print(5, y, max_width_str(name.first_name + std::string(" ") + name.last_name, 19), WHITE, background);
		term(1)->print(25, y, max_width_str(stats.profession_tag, 14), WHITE, background);
		term(1)->print(40, y, max_width_str(ai.job_status, 29), WHITE, background);
		++y;
	});
}

void panel_render_system::render_settler_mode() {
	term(1)->box(1, 2, 73, 60, WHITE, BLACK, true);
	for (int i=3; i<60; ++i) term(1)->print(2, i, "                                                                        ");

	name_t * name = entity(selected_settler)->component<name_t>();
	game_stats_t * stats = entity(selected_settler)->component<game_stats_t>();
	species_t * species = entity(selected_settler)->component<species_t>();
	settler_ai_t * ai = entity(selected_settler)->component<settler_ai_t>();

	std::stringstream header;
	header << name->first_name << " " << name->last_name << " (" << stats->profession_tag << ")";
	std::stringstream header2;
	header2 << species->gender_str() << ", " << species->sexuality_str() << ", " << stats->age << " years old. "
		<< species->height_feet() << ", " << species->weight_lbs();

	term(1)->print(2, 4, header.str(), YELLOW, BLACK );
	term(1)->print(2, 5, header2.str(), WHITE, BLACK );
	term(1)->print(2, 6, ai->job_status);
	term(1)->print(30, 8, species->gender_pronoun() + std::string(" ") + stats->strength_str(), WHITE, BLACK);
	term(1)->print(30, 9, species->gender_pronoun() + std::string(" ") + stats->dexterity_str(), WHITE, BLACK);
	term(1)->print(30, 10, species->gender_pronoun() + std::string(" ") + stats->constitution_str(), WHITE, BLACK);
	term(1)->print(30, 11, species->gender_pronoun() + std::string(" ") + stats->intelligence_str(), WHITE, BLACK);
	term(1)->print(30, 12, species->gender_pronoun() + std::string(" ") + stats->wisdom_str(), WHITE, BLACK);
	term(1)->print(30, 13, species->gender_pronoun() + std::string(" ") + stats->charisma_str(), WHITE, BLACK);
	term(1)->print(30, 14, species->gender_pronoun() + std::string(" ") + stats->comeliness_str(), WHITE, BLACK);
	term(1)->print(30, 15, species->gender_pronoun() + std::string(" ") + stats->ethics_str(), WHITE, BLACK);

	int y=17;
	for (const auto &skill : stats->skills) {
		term(1)->print(30, y, skill.first + std::string(" : ") + std::to_string(skill.second.skill_level));
	}
	++y;

	each<item_t, item_carried_t>([&y, this] (entity_t &e, item_t &item, item_carried_t &carried) {
		if (carried.carried_by == selected_settler) {
			term(1)->print(30, y, item.item_name);
			++y;
		}
	});

}

void panel_render_system::render_work_mode() {
	term(1)->box(1, 2, 73, 60, WHITE, BLACK, true);
	for (int i=3; i<60; ++i) term(1)->print(2, i, "                                                                        ");

	// List queued jobs
	int y = 3;
	for (const std::pair<uint8_t, std::string> &order : designations->build_orders) {
		std::stringstream ss;
		auto finder = reaction_defs.find(order.second);
		if (finder != reaction_defs.end()) {
			ss << std::setw(4) << +order.first << "  " << finder->second.name;
			term(1)->print(3, y, ss.str());
			++y;
		}
	}

	// List available tasks with "build it" button
	y = 3;
	auto available_reactions = get_available_reactions();
	for (const auto &reaction : available_reactions) {
		std::stringstream ss;
		ss << "[+] " << reaction.second;
		term(1)->print(40, y, ss.str());
		++y;
	}

	// Check for clicks to build
	if (get_mouse_button_state(rltk::button::LEFT) && mouse_damper > 20.0) {
		mouse_damper = 0.0;		
		int mouse_x, mouse_y;
		std::tie(mouse_x, mouse_y) = get_mouse_position();
		int terminal_x = mouse_x/8;
		int terminal_y = (mouse_y/8);

		if (terminal_x > 40 && terminal_y > 2 && terminal_y << y && terminal_x < 45) {
			const int selected_item = terminal_y - 3;
			const std::string tag = available_reactions[selected_item].first;

			bool found = false;
			for (auto &order : designations->build_orders) {
				if (order.second == tag) {
					++order.first;
					found = true;
				}
			}
			if (!found) designations->build_orders.push_back(std::make_pair(1, tag));
		}
	}

}