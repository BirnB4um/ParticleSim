#pragma once

#include <vector>
#include <memory>
#include <SFML/Graphics.hpp>
#include <iostream>

#define MAX_NODE_CAPACITY 40
#define MAX_NODE_DEPTH 15

class Quadtree
{
	

public:
	int m_level;
	std::vector<int> index_list;
	std::unique_ptr<Quadtree> subnodes[4];
	float min_x, max_x;
	float min_y, max_y;

	Quadtree(int level, float min_x, float max_x, float min_y, float max_y);
	void insert(int index, float x, float y);
	void split();
	void clear();
	bool contains_point(float x, float y);
	bool contains_rect(float rect_min_x, float rect_max_x, float rect_min_y, float rect_max_y);
	void draw(sf::RenderWindow &window);


};

