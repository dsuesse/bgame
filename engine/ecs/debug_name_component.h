#pragma once

#include "base_component.h"
#include <string>

using std::string;

namespace engine {
namespace ecs {

class debug_name_component : public base_component {
public:
    debug_name_component() { type = name; };
    debug_name_component(const string &NAME) : debug_name(NAME) { type = name; };
    string debug_name;
    
    virtual void save(fstream &lbfile) {
	base_component::save(lbfile);
	int name_size = debug_name.size();
	lbfile.write ( reinterpret_cast<const char *> ( &name_size ), sizeof ( name_size ) );
	lbfile.write ( debug_name.c_str(), name_size );
    };
    
    virtual void load(fstream &lbfile) {
	base_component::load(lbfile);
	int name_size = 0;
	lbfile.read ( reinterpret_cast<char *> ( &name_size ), sizeof ( name_size ) );
	char buf[name_size];
	lbfile.read ( buf, name_size );
	debug_name = string(buf);
    };
};
  
}
}
