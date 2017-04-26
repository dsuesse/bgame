#include "harvest_work.hpp"
#include "../../ai/path_finding.hpp"
#include "../../ai/inventory_system.hpp"
#include "../idle_mode.hpp"
#include "../settler_glyph.hpp"
#include "../settler_job_status.hpp"
#include "../settler_drop_tool.hpp"
#include "../settler_cancel_action.hpp"
#include "../pathfinding.hpp"
#include "../initiative.hpp"
#include "../../../raws/plants.hpp"
#include "../../../utils/telemetry.hpp"
#include "../../../main/game_designations.hpp"

using namespace rltk;
using tasks::become_idle;
using tasks::change_settler_glyph;
using tasks::change_job_status;
using tasks::drop_current_tool;
using tasks::cancel_action;
using tasks::follow_path;
using tasks::follow_result_t;
using tasks::calculate_initiative;

void do_harvesting(entity_t &e, settler_ai_t &ai, game_stats_t &stats, species_t &species, position_t &pos, name_t &name)
{
    if (ai.job_type_minor == JM_FIND_HARVEST) {
        position_t harvest_pos = position_t{ai.target_x, ai.target_y, ai.target_z};
        ai.current_path = find_path(pos, harvest_pos);
        if (!ai.current_path->success) {
            cancel_action(e, ai, stats, species, pos, name, "No route to harvest");
            return;
        }
        ai.job_type_minor = JM_GO_TO_HARVEST;
        change_job_status(ai, name, "Traveling to harvest.");
        return;
    }

    if (ai.job_type_minor == JM_GO_TO_HARVEST) {
        tasks::try_path(e, ai, pos,
                        [] () {}, // Do nothing on success
                        [&ai, &name] () {
                            ai.current_path.reset();
                            ai.job_type_minor = JM_DO_HARVEST;
                            change_job_status(ai, name, "Harvesting");
                        }, // On arrival
                        [&e, &ai, &stats, &species, &pos, &name] () {
                            cancel_action(e, ai, stats, species, pos, name, "No route to harvest");
                        }
        );
        return;
    }

    if (ai.job_type_minor == JM_DO_HARVEST) {
        // Remove the harvest designation
        const int idx = mapidx(ai.target_x, ai.target_y, ai.target_z);
        designations->harvest.erase(std::remove_if(
                designations->harvest.begin(),
                designations->harvest.end(),
                [&idx] (std::pair<bool,position_t> p) { return idx == mapidx(p.second); }
                                    ),
                                    designations->harvest.end());

        // Create the harvesting result
        if (current_region->tile_vegetation_type[idx] == 0) {
            cancel_action(e, ai, stats, species, pos, name, "Nothing to harvest");
            return;
        }
        const plant_t plant = get_plant_def(current_region->tile_vegetation_type[idx]);
        const std::string result = plant.provides[current_region->tile_vegetation_lifecycle[idx]];
        if (result != "none") {
            std::string mat_type = "organic";
            auto item_finder = item_defs.find(result);
            if (item_finder != item_defs.end()) {
                if (item_finder->second.categories.test(ITEM_FOOD)) mat_type="food";
                if (item_finder->second.categories.test(ITEM_SPICE)) mat_type="spice";
            }
            auto item = spawn_item_on_ground_ret(ai.target_x, ai.target_y, ai.target_z, result, get_material_by_tag(mat_type));
            item->component<item_t>()->item_name = plant.name;
        }

        // Knock tile back to germination
        current_region->tile_vegetation_lifecycle[idx] = 0;
        current_region->tile_vegetation_ticker[idx] = 0;

        // Become idle - done
        call_home("harvest", result);
        become_idle(e, ai, name);
        return;
    }
}
