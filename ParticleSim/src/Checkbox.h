#pragma once

#include <SFML/Graphics.hpp>
#include "LookupTable.h"

class Checkbox
{

private:
	float width, height;
	sf::Color fill_color;
	sf::Color border_color;

	bool is_mouse_over(float mouseX, float mouseY);

public:
	float pos_x, pos_y;
	bool is_checked;
	sf::RectangleShape rect_shape;

	Checkbox();
	Checkbox(float pos_x, float pos_y, float width, float height);
	~Checkbox();
	void set_position(float posX, float posY);
	void update(float mouseX, float mouseY);
	bool update_mouse_event(float mouseX, float mouseY, bool mouse_pressed);
	void update_state(bool checked);



};

