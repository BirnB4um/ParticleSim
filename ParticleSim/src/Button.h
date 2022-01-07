#pragma once

#include <SFML/Graphics.hpp>
#include "LookupTable.h"

class Button
{
private:
	sf::Color fill_color;
	sf::Color border_color;

	bool is_mouse_over(float mouseX, float mouseY);

public:
	float pos_x, pos_y;
	float width, height;
	bool was_pressed;
	bool is_pressed;
	bool mouse_over;
	sf::RectangleShape rect_shape;

	Button();
	Button(float pos_x, float pos_y, float width, float height);
	Button(float pos_x, float pos_y, float width, float height, sf::Color fillcolor, sf::Color bordercolor);
	~Button();
	void set_position(float posX, float posY);
	void update(float mouseX, float mouseY);
	bool update_mouse_event(float mouseX, float mouseY, bool mouse_pressed);



};

