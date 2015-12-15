#pragma once

#include "component_types.h"
#include "../world/location.hpp"
#include "position_component.h"

using namespace serialization_generic;

struct position_component3d {
     int entity_id;
     int handle;
     component_type type = position3d;
     bool deleted = false;

     position_component3d() {}
     position_component3d(const location_t &loc, const facing_t &fac) : facing(fac) { pos = loc; }

     facing_t facing;
     location_t pos;

     void load ( fstream &lbfile ) {
          load_common_component_properties<position_component3d> ( lbfile, *this );
	  int f=0;
	  load_primitive<int>( lbfile, f );
	  facing = static_cast<facing_t>(f);
	  load_primitive<location_t>( lbfile, pos );
     }

     void save ( fstream &lbfile ) {
          save_common_component_properties<position_component3d> ( lbfile, *this );
	  save_primitive<int> ( lbfile, facing );
	  save_primitive<location_t>( lbfile, pos);
     }
};
