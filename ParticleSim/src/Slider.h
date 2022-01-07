#pragma once

#include "SFML/Graphics.hpp"

class Slider
{

private:



public:
	float wert = 0.0f;//0-1
	float min_wert, max_wert;
	float width, height;
	float pos_x, pos_y;
	bool is_pressed;
	sf::RectangleShape rect_shape;
	sf::CircleShape circle_shape;
	
	Slider();
	Slider(float pos_x, float pos_y, float width, float height, float min_wert, float max_wert, float wert);
	Slider(float pos_x, float pos_y, float width, float height, float circle_radius, float min_wert, float max_wert, float wert);
	~Slider();
	float get_value();
	void set_position(float posX, float posY);
	void update(float mouseX, float mouseY);
	bool update_mouse_event(float mouseX, float mouseY, bool is_button_pressed);
	void update_state(float new_wert);
	bool mouse_over_circle(float mouseX, float mouseY);

};

