#include "Quadtree.h"


Quadtree::Quadtree(int level, float min_x, float max_x, float min_y, float max_y)
	: m_level(level),
	subnodes{nullptr, nullptr, nullptr, nullptr},
	min_x(min_x),
	max_x(max_x),
	min_y(min_y),
	max_y(max_y){

	index_list.reserve(MAX_NODE_CAPACITY);
}

void Quadtree::insert(int index, float x, float y) {

	//if not split
	if (subnodes[0] != nullptr) {
		if (subnodes[0]->contains_point(x, y))subnodes[0]->insert(index, x, y);
		if (subnodes[1]->contains_point(x, y))subnodes[1]->insert(index, x, y);
		if (subnodes[2]->contains_point(x, y))subnodes[2]->insert(index, x, y);
		if (subnodes[3]->contains_point(x, y))subnodes[3]->insert(index, x, y);
		return;
	}

	if (contains_point(x, y)) {
		if (index_list.size() < MAX_NODE_CAPACITY || m_level == MAX_NODE_DEPTH) {
			index_list.push_back(index);
			return;
		}
		split();
		insert(index, x, y);
	}

}



void Quadtree::split() {

	float mid_x = min_x + (max_x - min_x) / 2;
	float mid_y = min_y + (max_y - min_y) / 2;

	subnodes[0] = std::make_unique<Quadtree>(m_level+1, min_x, mid_x, min_y, mid_y);//NW
	subnodes[1] = std::make_unique<Quadtree>(m_level+1, mid_x, max_x, min_y, mid_y);//NO
	subnodes[2] = std::make_unique<Quadtree>(m_level+1, min_x, mid_x, mid_y, max_y);//SW
	subnodes[3] = std::make_unique<Quadtree>(m_level+1, mid_x, max_x, mid_y, max_y);//SO
}

bool Quadtree::contains_point(float x, float y) {
	return x >= min_x && x <= max_x && y >= min_y && y <= max_y;
}

bool Quadtree::contains_rect(float rect_min_x, float rect_max_x, float rect_min_y, float rect_max_y) {
	return ( (rect_min_x >= min_x && rect_min_x <= max_x) || (rect_max_x >= min_x && rect_max_x <= max_x)) && ((rect_min_y >= min_y && rect_min_y <= max_y) || (rect_max_y >= min_y && rect_max_y <= max_y) );
}

void Quadtree::clear() {
	subnodes[0] = nullptr;
	subnodes[1] = nullptr;
	subnodes[2] = nullptr;
	subnodes[3] = nullptr;

	index_list.clear();

}

void Quadtree::draw(sf::RenderWindow& window) {
	sf::RectangleShape rect;
	rect.setSize(sf::Vector2f(max_x - min_x, max_y - min_y));
	rect.setPosition(min_x, min_y);
	rect.setOutlineThickness(1);
	rect.setFillColor(sf::Color::Transparent);
	rect.setOutlineColor(sf::Color::White);
	window.draw(rect);

	if (subnodes[0] != nullptr) {
		subnodes[0]->draw(window);
		subnodes[1]->draw(window);
		subnodes[2]->draw(window);
		subnodes[3]->draw(window);
	}
}
