#include "Checkbox.h"

Checkbox::Checkbox() {
	this->is_checked = false;
	this->pos_x = 0;
	this->pos_y = 0;
	this->width = 100;
	this->height = 100;
	this->fill_color = sf::Color(20, 20, 20);
	this->border_color = sf::Color::White;

	rect_shape.setSize(sf::Vector2f(width, height));
	rect_shape.setPosition(sf::Vector2f(pos_x, pos_y));
	rect_shape.setFillColor(fill_color);
	rect_shape.setOutlineThickness(4);
	rect_shape.setOutlineColor(border_color);
}

Checkbox::Checkbox(float pos_x, float pos_y, float width, float height) {
	this->is_checked = false;
	this->pos_x = pos_x;
	this->pos_y = pos_y;
	this->width = width;
	this->height = height;
	this->fill_color = sf::Color(20, 20, 20);
	this->border_color = sf::Color::White;

	rect_shape.setSize(sf::Vector2f(width, height));
	rect_shape.setPosition(sf::Vector2f(pos_x, pos_y));
	rect_shape.setFillColor(fill_color);
	rect_shape.setOutlineThickness(4);
	rect_shape.setOutlineColor(border_color);
}

Checkbox::~Checkbox() {

}


bool Checkbox::is_mouse_over(float mouseX, float mouseY) {
	if (mouseX > rect_shape.getPosition().x && mouseX < rect_shape.getPosition().x + width &&
		mouseY > rect_shape.getPosition().y && mouseY < rect_shape.getPosition().y + height) {
		return true;
	}
	return false;
}

void Checkbox::update(float mouseX, float mouseY) {

}

void Checkbox::set_position(float posX, float posY) {
	rect_shape.setPosition(sf::Vector2f(posX, posY));
	this->pos_x = posX;
	this->pos_y = posY;
}


bool Checkbox::update_mouse_event(float mouseX, float mouseY, bool mouse_pressed) {
	if (is_mouse_over(mouseX, mouseY) && !mouse_pressed) {
		is_checked = !is_checked;

		if (is_checked) {
			rect_shape.setFillColor(sf::Color(150, 150, 150));
		}
		else {
			rect_shape.setFillColor(fill_color);
		}
		return true;
	}
	return false;
}

void Checkbox::update_state(bool checked) {
	is_checked = checked;

	if (is_checked) {
		rect_shape.setFillColor(sf::Color(150, 150, 150));
	}
	else {
		rect_shape.setFillColor(fill_color);
	}
}
