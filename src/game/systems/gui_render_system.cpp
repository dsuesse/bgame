#include "gui_render_system.h"
#include "../game.h"
#include "../world/universe.hpp"
#include "render_ascii.hpp"
#include "gui/gui_static_text.hpp"
#include "gui/gui_popup_menu.hpp"
#include "../tasks/path_finding.h"
#include "../raws/raws.h"
#include "../world/inventory.hpp"
#include "../raws/raw_input.h"
#include "../raws/raw_output.h"
#include "../raws/raw_power_drain.h"
#include "../raws/raw_emote.h"
#include <memory>
#include <sstream>
#include <vector>

using std::stringstream;

void gui_render_system::render_cursor(engine::sdl2_backend * SDL, std::pair<int, int> &screen_size, SDL_Rect &viewport, position_component3d * camera_pos)
{
	int mouse_x = engine::command::mouse_x;
	int mouse_y = engine::command::mouse_y;

	if (mouse_x > 0 and mouse_x < screen_size.first and mouse_y > 48 and mouse_y < screen_size.second)
	{
		const int tile_x = mouse_x / 8;
		const int tile_y = (mouse_y - 48) / 8;

		const int16_t tilespace_x = tile_x + viewport.x;
		const int16_t tilespace_y = tile_y + viewport.y;

		if (tilespace_x == last_mouse_x and tilespace_y == last_mouse_y)
		{
			++mouse_hover_time;
		}
		else
		{
			mouse_hover_time = 0;
		}
		last_mouse_x = tilespace_x;
		last_mouse_y = tilespace_y;

		const location_t target	{ camera_pos->pos.region, tilespace_x, tilespace_y,	camera_pos->pos.z };
		const int target_idx = get_tile_index(tilespace_x, tilespace_y,	camera_pos->pos.z);

		engine::vterm::screen_character cursor{ 176, color_t{ 255, 255, 0 }, color_t{ 0, 0, 0 } };
		if (!world::planet->get_region(target.region)->revealed[target_idx])
		{
			cursor.foreground_color = color_t{ 64, 64, 64 };
		}
		else
		{
			tile_t * target_tile = world::planet->get_tile(target);
			if (target_tile->flags.test(TILE_OPTIONS::SOLID))
			{
				cursor.foreground_color = color_t{ 255, 0, 0 };
			}
			if (target_tile->flags.test(TILE_OPTIONS::CAN_STAND_HERE))
			{
				cursor.foreground_color = color_t{ 0, 255, 255 };
			}
		}

		int alpha = 32 + (mouse_hover_time * 8);
		if (alpha > 200) alpha = 200;
		SDL->set_alpha_mod("font_s", alpha);
		SDL_Rect dest{ tile_x * 8, (tile_y * 8) + 48, 8, 8 };
		render_ascii(dest, cursor, SDL, 0, make_tuple(1.0,1.0,1.0),true);
		SDL->set_alpha_mod("font_s", 255);

		if (world::planet->get_region(camera_pos->pos.region)->revealed[target_idx])
		{
			tile_t * target_tile = world::planet->get_tile(target);
			{
				stringstream ss;
				if (target_tile->flags.test(TILE_OPTIONS::SOLID)) ss << "SOLID / ";
				ss << tile_type_to_string(target_tile->base_tile_type) << " / ";
				if (target_tile->base_tile_type != tile_type::EMPTY_SPACE)
				{
					ss << ground_type_to_string(target_tile->ground) << " / ";
					ss << covering_type_to_string(target_tile->covering) << " / ";
				}
				ss << climate_type_to_string(target_tile->climate);

				gui_element * tooltip1 = gui.get_element("tooltip1");
				if (tooltip1 == nullptr) {
					gui.add_element("tooltip1", std::make_unique<gui_static_text>( "disco10", ss.str(), render::sdl_white, 10, screen_size.second-10 ));
				} else {
					gui_static_text * tt1 = static_cast<gui_static_text *>(tooltip1);
					tt1->message = ss.str();
				}

				//std::string line_s = SDL->render_text_to_image("disco10", ss.str(), "tmp", render::sdl_white);
				//SDL->render_bitmap_simple(line_s, 10, screen_size.second-10);
			}

			// Add tile contents
			stringstream ss;

			int region_x = tilespace_x;
			int region_y = tilespace_y;
			int region_z = camera_pos->pos.z;
			vector<position_component3d *> positions = ECS->find_components_by_func<position_component3d>([&ss, region_x, region_y, region_z] ( const position_component3d &c )
			{
				return ( c.pos.x == region_x and c.pos.y == region_y and c.pos.z == region_z );
			});
			for (const position_component3d * pos : positions)
			{
				const int entity_id = pos->entity_id;

				const settler_ai_component * settler = ECS->find_entity_component<settler_ai_component>(entity_id);
				if ( settler != nullptr) {
					ss << settler->first_name << " " << settler->last_name << "  ";
				} else {
					debug_name_component * name = ECS->find_entity_component<debug_name_component>(entity_id);
					if (name != nullptr) ss << name->debug_name << "(" << name->entity_id << ") ";
				}

				vector<item_storage_component *> stored_items =	ECS->find_components_by_func<item_storage_component>(
									[entity_id] ( const item_storage_component &e )
									{
										if ( e.container_id == entity_id )
										{
											return true;
										}
										else
										{
											return false;
										}
									});
					for (item_storage_component * item : stored_items)
					{
						debug_name_component * nc = ECS->find_entity_component<debug_name_component>(item->entity_id);
						ss << "[" << nc->debug_name << "]  ";
					}
			}

			if (ss.str().size() < 1) {
				ss << "Nothing interesting here.";
			}
			gui_element * tooltip2 = gui.get_element("tooltip2");
			if (tooltip2 == nullptr) {
				gui.add_element("tooltip2", std::make_unique<gui_static_text>("disco10", ss.str(), render::sdl_white, 10, screen_size.second+2));
			} else {
				gui_static_text * tt2 = static_cast<gui_static_text *>(tooltip2);
				tt2->message = ss.str();
			}

		}
	}
}

void gui_render_system::tick(const double& duration_ms) {
	sdl2_backend * SDL = game_engine->get_backend();

	// Calculations - these are duplicated in map_render and should be refactored to be done once.
	position_component3d * camera_pos = get_camera_position();
	region_t * current_region = world::planet->get_region(camera_pos->pos.region);
	pair<int, int> screen_size = SDL->get_screen_size();
	screen_size.second -= 48;
	const int ascii_width = screen_size.first / 8;
	const int ascii_height = screen_size.second / 8;
	if (camera_pos->pos.x < ascii_width / 2) camera_pos->pos.x = ascii_width / 2;
	if (camera_pos->pos.x > REGION_WIDTH - (ascii_width / 2)) camera_pos->pos.x = REGION_WIDTH - (ascii_width / 2);
	if (camera_pos->pos.y < ascii_height / 2) camera_pos->pos.y = ascii_height / 2;
	if (camera_pos->pos.y > REGION_HEIGHT - (ascii_height / 2))	camera_pos->pos.y = REGION_HEIGHT - (ascii_height / 2);
	SDL_Rect viewport{ camera_pos->pos.x - (ascii_width / 2), camera_pos->pos.y	- (ascii_height / 2), ascii_width, ascii_height };

	// Render mode
	if (mode == normal and engine::command::right_click and mouse_x > 0 and mouse_x < screen_size.first and mouse_y > 48 and mouse_y < screen_size.second) {
		mode = radial;
		const int tile_x = mouse_x / 8;
		const int tile_y = (mouse_y - 48) / 8;

		const int16_t tilespace_x = tile_x + viewport.x;
		const int16_t tilespace_y = tile_y + viewport.y;

		radial_screen_x = mouse_x;
		radial_screen_y = mouse_y;
		radial_tilespace_x = tilespace_x;
		radial_tilespace_y = tilespace_y;
		radial_tilespace_z = camera_pos->pos.z;

		universe->globals.paused = true;

		tile_t * target_tile = world::planet->get_tile(location_t{camera_pos->pos.region,
			static_cast<int16_t>(radial_tilespace_x), static_cast<int16_t>(radial_tilespace_y), static_cast<uint8_t>(radial_tilespace_z)});

		// Launch the pop-up menu
		std::unique_ptr<gui_popup_menu> menu = std::make_unique<gui_popup_menu>("TILE OPTIONS", radial_screen_x, radial_screen_y);

		bool added_something = false;

		std::vector<position_component3d> * positions = ECS->find_components_by_type<position_component3d>();
		for (const position_component3d &pos : *positions) {
			if (pos.pos.x == radial_tilespace_x and pos.pos.y == radial_tilespace_y and pos.pos.z == radial_tilespace_z) {
				const int entity_id = pos.entity_id;

				// Is there a settler here? If so, they get an option
				settler_ai_component * settler = ECS->find_entity_component<settler_ai_component>(entity_id);
				if (settler != nullptr) {
					added_something = true;
					std::stringstream ss;
					ss << settler->first_name << " " << settler->last_name << " (" << settler->profession_tag << ")";
					menu->add_option(gui_menu_option{ ss.str(), [] {
						std::cout << "Settler info requested\n";
					} });
				}

				// Is there a structure here? If so, it gets an option
				if (!added_something) {
					// TODO: Add names/descriptions, reactions, chopping, deconstruction if nothing already here
					tree_component * tree = ECS->find_entity_component<tree_component>(entity_id);
					if (tree != nullptr) {
						std::stringstream ss;
						ss << "Chop down tree #" << entity_id;
						int TREE = entity_id;
						menu->add_option(gui_menu_option{ ss.str(), [=] {
							std::cout << "Chopper #" << TREE << "\n";
							game_engine->messaging->add_message<chop_order_message> ( chop_order_message ( this->radial_tilespace_x, this->radial_tilespace_y, this->radial_tilespace_z, TREE, "Chop Tree" ) );
							std::cout << "Chopper #" << TREE << "\n";
						}});
						added_something = true;
					}

					// Look for reactions
					debug_name_component * name = ECS->find_entity_component<debug_name_component>(entity_id);
					if (name != nullptr) {
						vector<raws::base_raw *> reactions = raws::get_possible_reactions_for_structure(name->debug_name);
						for (raws::base_raw * br : reactions)
						{
							vector<pair<string, int>> required_components;
							vector<pair<string, int>> creates;
							int power_drain = 0;
							string emote;
							string reaction_name;
							int output_idx = 0;

							for (const std::unique_ptr<raws::base_raw> &brc : br->children)
							{
								if (brc->type == raws::INPUT)
								{
									raws::raw_input * input = static_cast<raws::raw_input *>(brc.get());
									required_components.push_back(std::make_pair(input->required_item, input->quantity));
								}
								if (brc->type == raws::OUTPUT)
								{
									raws::raw_output * input = static_cast<raws::raw_output *>(brc.get());
									creates.push_back(std::make_pair(input->created_item, input->quantity));
								}
								if (brc->type == raws::POWER_DRAIN)
								{
									raws::raw_power_drain * input =	static_cast<raws::raw_power_drain *>(brc.get());
									power_drain = input->quantity;
								}
								if (brc->type == raws::EMOTE)
								{
									raws::raw_emote * input = static_cast<raws::raw_emote *>(brc.get());
									emote = input->emote;
								}
								if (brc->type == raws::NAME)
								{
									reaction_name = brc->get_name_override();
								}
							}

							bool possible = true;
							for (const std::pair<string, int> &component : required_components)
							{
								auto finder = inventory.find(component.first);
								if (finder == inventory.end())
								{
									possible = false;
								}
								else
								{
									const int available_components = finder->second.size();
									if (available_components < component.second)
										possible = false;
								}
							}

							if (possible)
							{
								menu->add_option(gui_menu_option{reaction_name,
										[=]
										{
											game_engine->messaging->add_message<reaction_order_message>( reaction_order_message( reaction_name, entity_id, required_components, creates, emote, power_drain ) );
										}
								});
								added_something = true;
							}
							else
							{
								std::cout << "Reaction " << reaction_name << " was ruled impossible.\n";
							}
						}
					}
				}
			}
		}

		if (!added_something) {
			// Can we build here? If so, then buildables go here
			bool can_build = true;
			// Only allow building if we can get to the tile or an adjacent tile
			position_component3d * cordex_position = ECS->find_entity_component<position_component3d>(universe->globals.cordex_handle);
			std::shared_ptr<ai::navigation_path> path = ai::find_path(cordex_position->pos, camera_pos->pos);
			if ( path->success == false ) {
				std::cout << "Excluding because of lack of path.\n";
				can_build = false;
			}

			// TODO: More checking on position - can you stand there, etc.
			if (target_tile->flags.test(TILE_OPTIONS::SOLID)) can_build = false;
			if (target_tile->flags.test(TILE_OPTIONS::WALK_BLOCKED)) can_build = false;
			if (target_tile->flags.test(TILE_OPTIONS::CAN_STAND_HERE)==false) can_build = false;

			if (can_build) {
				// Determine if there is anything we can build
				vector<tuple<string, int, vector<string>>> buildables = raws::get_buildables();
				for (const tuple<string, int, vector<string>> &buildable : buildables) {
					// Check that we have its components
					bool have_components = true;
					int available = 0;

					for (const string &component : std::get<2>(buildable))
					{
						auto finder = inventory.find(component);
						if (finder == inventory.end())
						{
							have_components = false;
						}
						else
						{
							available = finder->second.size();
						}
					}

					// If we're still good - add it
					if (have_components) {
						added_something = true;
						std::stringstream ss;
						ss << std::get<0>(buildable);
						ss << ", using: ";
						for (const string &component : std::get<2>(buildable)) {
							ss << component << " ";
						}
						ss << " <" << available << ">";
						menu->add_option(gui_menu_option{ss.str(), [=] {
							game_engine->messaging->add_message<build_order_message> ( build_order_message ( radial_tilespace_x, radial_tilespace_y, radial_tilespace_z, std::get<0>(buildable) ) );
						}});
					}
				}
			}
		}

		if (!added_something and target_tile->flags.test(TILE_OPTIONS::SOLID)) {
			// Can you path to an adjacent tile?
			menu->add_option(gui_menu_option{"Dig out tile", [=] {
				game_engine->messaging->add_message<dig_order_message> ( dig_order_message ( radial_tilespace_x, radial_tilespace_y, radial_tilespace_z ) );
			}});
			menu->add_option(gui_menu_option{"Channel out tile", [=] {
				game_engine->messaging->add_message<dig_order_message> ( dig_order_message ( radial_tilespace_x, radial_tilespace_y, radial_tilespace_z, dig_types::CHANNEL ) );
			}});
			menu->add_option(gui_menu_option{"Dig upwards ramp", [=] {
				game_engine->messaging->add_message<dig_order_message> ( dig_order_message ( radial_tilespace_x, radial_tilespace_y, radial_tilespace_z, dig_types::RAMP ) );
			}});
			menu->add_option(gui_menu_option{"Dig downwards staircase", [=] {
				game_engine->messaging->add_message<dig_order_message> ( dig_order_message ( radial_tilespace_x, radial_tilespace_y, radial_tilespace_z, dig_types::DOWNSTAIRS ) );
			}});
			menu->add_option(gui_menu_option{"Dig upwards staircase", [=] {
				game_engine->messaging->add_message<dig_order_message> ( dig_order_message ( radial_tilespace_x, radial_tilespace_y, radial_tilespace_z, dig_types::UPSTAIRS ) );
			}});
			menu->add_option(gui_menu_option{"Dig up/down stairs", [=] {
				game_engine->messaging->add_message<dig_order_message> ( dig_order_message ( radial_tilespace_x, radial_tilespace_y, radial_tilespace_z, dig_types::UPDOWNSTAIRS ) );
			}});
			added_something = true;
		}

		if (!added_something) {
			menu->add_option(gui_menu_option{"Nothing to do here"});
		}
		gui.add_element("popup_menu", std::move(menu));
	}

	if (mode == normal)
	{
		render_cursor(SDL, screen_size, viewport, camera_pos);
	}
	else if (mode == radial)
	{
		universe->globals.render_flat = true;
		if (!universe->globals.paused)
		{
			mode = normal;
			gui.delete_element("popup_menu");
		}
	}
	gui.render();
	if (mode == radial and gui.get_element("popup_menu")==nullptr) {
		mode = normal;
	}
}