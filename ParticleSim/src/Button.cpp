#include "Button.h"


Button::Button() {
	this->was_pressed = false;
	this->mouse_over = false;
	this->is_pressed = false;
	this->pos_x = 0;
	this->pos_y = 0;
	this->width = 100;
	this->height = 100;
	this->fill_color = sf::Color(20, 20, 20);
	this->border_color = sf::Color::White;

	rect_shape.setSize(sf::Vector2f(width, height));
	rect_shape.setPosition(sf::Vector2f(pos_x, pos_y));
	rect_shape.setFillColor(fill_color);
	rect_shape.setOutlineThickness(2);
	rect_shape.setOutlineColor(border_color);
}
Button::Button(float pos_x, float pos_y, float width, float height) {
	this->was_pressed = false;
	this->mouse_over = false;
	this->is_pressed = false;
	this->pos_x = pos_x;
	this->pos_y = pos_y;
	this->width = width;
	this->height = height;
	this->fill_color = sf::Color(20, 20, 20);;
	this->border_color = sf::Color::White;

	rect_shape.setSize(sf::Vector2f(width, height));
	rect_shape.setPosition(sf::Vector2f(pos_x, pos_y));
	rect_shape.setFillColor(fill_color);
	rect_shape.setOutlineThickness(4);
	rect_shape.setOutlineColor(border_color);
}
Button::Button(float pos_x, float pos_y, float width, float height, sf::Color fillcolor, sf::Color bordercolor) {
	this->was_pressed = false;
	this->mouse_over = false;
	this->is_pressed = false;
	this->pos_x = pos_x;
	this->pos_y = pos_y;
	this->width = width;
	this->height = height;
	this->fill_color = fillcolor; 
	this->border_color = bordercolor;

	rect_shape.setSize(sf::Vector2f(width, height));
	rect_shape.setPosition(sf::Vector2f(pos_x, pos_y));
	rect_shape.setFillColor(fillcolor);
	rect_shape.setOutlineThickness(4);
	rect_shape.setOutlineColor(bordercolor);
}

Button::~Button() {

}


bool Button::is_mouse_over(float mouseX, float mouseY) {
	if (mouseX > rect_shape.getPosition().x && mouseX < rect_shape.getPosition().x + width &&
		mouseY > rect_shape.getPosition().y && mouseY < rect_shape.getPosition().y + height) {
		return true;
	}
	return false;
}

void Button::update(float mouseX, float mouseY) {

	is_mouse_over(mouseX, mouseY) ? mouse_over = true : mouse_over = false;

	if (is_pressed) {
		rect_shape.setFillColor(sf::Color(180, 180, 180));

		if (!was_pressed) {
			//std::cout << "pressed" << std::endl;
		}

		if (is_pressed)
			was_pressed = true;
	}
	else {
		if (mouse_over) {
			rect_shape.setFillColor(sf::Color(60, 60, 60));
		}
		else {
			rect_shape.setFillColor(fill_color);
		}

		if (was_pressed) {
			//std::cout << "released" << std::endl;
		}

		if (!is_pressed)
			was_pressed = false;
	}

}

void Button::set_position(float posX, float posY) {
	rect_shape.setPosition(sf::Vector2f(posX, posY));
	this->pos_x = posX;
	this->pos_y = posY;
}


bool Button::update_mouse_event(float mouseX, float mouseY, bool mouse_pressed) {
	if (is_mouse_over(mouseX, mouseY)) {
		is_pressed = mouse_pressed;
		return true;
	}
	is_pressed = false;
	return false;
}