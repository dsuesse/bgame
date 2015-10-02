#include "engine/engine.h"
#include <sstream>

using namespace engine;
using std::make_pair;

class test_mode : public engine::base_mode {
public:
     virtual void init() { }
     virtual void done() { }
     virtual pair<return_mode, unique_ptr<base_mode>> tick ( const double time_elapsed ) {
          std::stringstream tmp;
          tmp << "Hello World, Iteration " << count;
	  tuple<unsigned char,unsigned char,unsigned char> fg{255,0,roll_dice(1,255)};
	  tuple<unsigned char,unsigned char,unsigned char> bg{0,0,count*2};
          vterm::print ( 0, y, tmp.str(), fg, bg );
          ++y;
          if ( y > 20 ) y = 0;
          ++count;

          if ( count < 100 ) {
               return make_pair ( CONTINUE, NO_PUSHED_MODE );
          } else {
               return make_pair ( POP, NO_PUSHED_MODE );
          }
     }
private:
     int y=0;
     int count = 0;
};
