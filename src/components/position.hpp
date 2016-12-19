#pragma once

#include <rltk.hpp>

using namespace rltk;

struct position_t {
	float x=0;
	float y=0;
	int z=0;

	position_t() {}
	position_t(const int &X, const int &Y, const int &Z) : x(static_cast<float>(X)), y(static_cast<float>(Y)), z(Z) {}
    position_t(const float &X, const float &Y, const int &Z) : x(X), y(Y), z(Z) {}
	bool operator==(position_t &rhs) { return (x==rhs.x && y==rhs.y && z==rhs.z); }

	std::string xml_identity = "position_t";

	void to_xml(xml_node * c);
	void from_xml(xml_node * c);
};