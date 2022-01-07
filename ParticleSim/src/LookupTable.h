#pragma once
#include <iostream>
#include <SFML/Graphics.hpp>
#include <windows.h>
#include <fstream>

class LookupTable
{
	

private:

public:
	int force_functions_count;
	int size_one_partricle_type = 0;
	int lookupData_size = 0;
	float* lookupData;
	int particle_types_count;
	int particle_count;
	float max_influence_radius;
	float max_force;
	float max_velocity;
	float friction;
	float particle_radius;
	bool wrap_around_screen;
	int max_particle_count;
	int max_particle_type_count;
	float max_particle_radius;

public:
	LookupTable();
	~LookupTable();
	void init(unsigned int seed);
	void randomise_table(unsigned int seed);
	void resize_table(unsigned int seed, int types_count);


	sf::Color get_Color(int particle_id);
	int get_raw_Color_as_int(int particle_id);
	float get_normalised_force_from_function(int force_function_id, float radius);
	float get_max_force(int stationary_particle_id, int attracted_particle_id);
	float get_influence_radius(int stationary_particle_id, int attracted_particle_id);
	float get_force_from_function(int stationary_particle_id, int attracted_particle_id, float distance);
	void randomise_color(int seed);

};