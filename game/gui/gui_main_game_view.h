#include "../../engine/gui/gui_element_base.h"
#include "../../game/world/world.h"
#include <string>
#include <iostream>
#include <algorithm>
#include "../../engine/ecs.h"
#include "../../game/components/position_component.h"
#include <cmath>

using std::string;

namespace engine {
class gui_main_game_view : public gui_element_base {
public:
     gui_main_game_view ( const string name ) {
          tag = name;
     };

     virtual void render ( const screen_region viewport );

private:
     void render_heading( const engine::screen_region& viewport, const int& vp_left, const int& vp_right, const int& vp_top, const int& vp_bottom );
};
}
