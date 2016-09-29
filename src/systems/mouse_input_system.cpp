#include "mouse_input_system.hpp"

namespace mouse {
	int x, y;
	int font1_w, font1_h;
	int term1x;
	int term1y;
	bool clicked;
}

void mouse_input_system::configure() {
    system_name = "Mouse Input System";
}

void mouse_input_system::update(const double ms) {
	std::tie(mouse::x, mouse::y) = rltk::get_mouse_position();
	std::tie(mouse::font1_w, mouse::font1_h) = rltk::term(1)->get_font_size();
	mouse::term1x = mouse::x / mouse::font1_w;
	mouse::term1y = mouse::y / mouse::font1_h;
	mouse::clicked = rltk::get_mouse_button_state(rltk::button::LEFT);
}
