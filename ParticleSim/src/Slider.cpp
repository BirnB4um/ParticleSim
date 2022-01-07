#include "Slider.h"

Slider::Slider() {
	this->pos_x = 0;
	this->pos_y = 0;
	this->width = 100;
	this->height = 100;
	this->min_wert = 0.0f;
	this->max_wert = 1.0f;
	this->wert = 0.0f;
	this->is_pressed = false;

	rect_shape.setSize(sf::Vector2f(width, height));
	rect_shape.setPosition(sf::Vector2f(pos_x, pos_y));
	rect_shape.setFillColor(sf::Color(100, 100, 100, 255));
	rect_shape.setOutlineColor(sf::Color(100, 100, 100, 255));

	float r = 20.0f;
	circle_shape.setRadius(r);
	circle_shape.setOrigin(r, r);
	circle_shape.setPosition(pos_x, pos_y + height / 2);
	circle_shape.setFillColor(sf::Color(80, 80, 80, 255));

}

Slider::Slider(float pos_x, float pos_y, float width, float height, float min_wert, float max_wert, float wert) {
	this->pos_x = pos_x;
	this->pos_y = pos_y;
	this->width = width;
	this->height = height;
	this->min_wert = min_wert;
	this->max_wert = max_wert;
	this->wert = wert;
	this->is_pressed = false;

	rect_shape.setSize(sf::Vector2f(width, height));
	rect_shape.setPosition(sf::Vector2f(pos_x, pos_y));
	rect_shape.setFillColor(sf::Color(100, 100, 100, 255));
	rect_shape.setOutlineColor(sf::Color(100, 100, 100, 255));

	float r = 20.0f;
	circle_shape.setRadius(r);
	circle_shape.setOrigin(r, r);
	circle_shape.setPosition(pos_x, pos_y + height/2);
	circle_shape.setFillColor(sf::Color(120, 120, 120, 255));
}

Slider::Slider(float pos_x, float pos_y, float width, float height,float circle_radius, float min_wert, float max_wert, float wert) {
	this->pos_x = pos_x;
	this->pos_y = pos_y;
	this->width = width;
	this->height = height;
	this->min_wert = min_wert;
	this->max_wert = max_wert;
	this->wert = wert;
	this->is_pressed = false;

	rect_shape.setSize(sf::Vector2f(width, height));
	rect_shape.setPosition(sf::Vector2f(pos_x, pos_y));
	rect_shape.setFillColor(sf::Color(100, 100, 100, 255));
	rect_shape.setOutlineColor(sf::Color(100, 100, 100, 255));

	circle_shape.setRadius(circle_radius);
	circle_shape.setOrigin(circle_radius, circle_radius);
	circle_shape.setPosition(pos_x, pos_y + height / 2);
	circle_shape.setFillColor(sf::Color(120, 120, 120, 255));
}

Slider::~Slider() {

}

float Slider::get_value() {
	return min_wert + wert * (max_wert - min_wert);
}

void Slider::set_position(float posX, float posY) {
	pos_x = posX;
	pos_y = posY;
	rect_shape.setPosition(posX, posY);
	circle_shape.setPosition(rect_shape.getPosition().x + wert * width, rect_shape.getPosition().y);
}

bool Slider::mouse_over_circle(float mouseX, float mouseY) {
	return mouseX > pos_x - circle_shape.getRadius() && mouseX < pos_x + width + circle_shape.getRadius() && mouseY > pos_y - circle_shape.getRadius() && mouseY < pos_y + circle_shape.getRadius();
	//return mouseX > circle_shape.getPosition().x - circle_shape.getRadius() && mouseX < circle_shape.getPosition().x + circle_shape.getRadius() && mouseY > circle_shape.getPosition().y - circle_shape.getRadius() && mouseY < circle_shape.getPosition().y + circle_shape.getRadius();
}

bool Slider::update_mouse_event(float mouseX, float mouseY, bool is_button_pressed) {
	if (is_button_pressed) {
		if(mouse_over_circle(mouseX, mouseY)){
			is_pressed = true;
			return true;
		}
	}
	else {
		is_pressed = false;
	}
	return false;
}

void Slider::update(float mouseX, float mouseY) {
	if (is_pressed) {
		circle_shape.setPosition(  mouseX < pos_x ? pos_x : mouseX > pos_x + width ? pos_x + width : mouseX  , circle_shape.getPosition().y);
		wert = (circle_shape.getPosition().x - pos_x) / width;
	}
	circle_shape.setPosition(pos_x + width * wert, pos_y);
}

void Slider::update_state(float new_wert) {
	new_wert = new_wert < min_wert ? min_wert : new_wert > max_wert ? max_wert : new_wert;//keep in bounds
	wert = (new_wert - min_wert) / (max_wert - min_wert);//set wert
	circle_shape.setPosition(pos_x + width * wert, pos_y);//update circle positioin
}
