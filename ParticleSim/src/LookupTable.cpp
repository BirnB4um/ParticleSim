#include "LookupTable.h"


LookupTable::LookupTable() {
}

LookupTable::~LookupTable() {
	delete[] lookupData;
}

void LookupTable::init(unsigned int seed) {
	particle_radius = 8.0f;
	wrap_around_screen = true;
	particle_count = 200;
	particle_types_count = 5;
	max_particle_count = 100000;
	max_particle_type_count = 30;
	max_particle_radius = 32.0f;

	friction = 0.99f;
	max_influence_radius = 500.0f;
	max_force = 0.2f;
	max_velocity = 16.0f;
	force_functions_count = 6;//constant, linear falling, linear rising, sinus, double linear, mountain

	resize_table(seed, particle_types_count);
	randomise_color(seed);
}


void LookupTable::resize_table(unsigned int seed, int types_count) {
	delete[] lookupData;
	particle_types_count = types_count;

	size_one_partricle_type = (particle_types_count * 3 + 1);
	lookupData_size = particle_types_count * size_one_partricle_type;
	lookupData = new float[lookupData_size];
	memset(lookupData, 0, lookupData_size * sizeof(float));//clear data

	randomise_table(seed);
}

void LookupTable::randomise_table(unsigned int seed) {
	std::srand(seed);
	float min_wert = 0.4f;
	float wert = 0.0f;
	for (int i = 0; i < particle_types_count; i++) {//for every particle

		for (int n = 0; n < 3; n++) {//0:forceFunc, 1:maxForce_multiplier, 2:maxRadius_multiplier
			for (int k = 0; k < particle_types_count; k++) {//for every other particle

				if (n == 0) {//forceFunc (0 - count_force_functions)
					lookupData[i * size_one_partricle_type + n * particle_types_count + k] =   float(std::rand() % force_functions_count);
				}
				else if (n == 1) {//force multiplier (-1, 1) 
					lookupData[i * size_one_partricle_type + n * particle_types_count + k] = ((std::rand() % 2) * 2 - 1); //max_force *((std::rand() % 3) - 1);
				}
				else if (n == 2) {//radius multiplier (min_wert - 1)
					wert = ((double)rand() / (RAND_MAX));
					lookupData[i * size_one_partricle_type + n * particle_types_count + k] = (wert < min_wert ? wert + min_wert : wert);
				}
			}
		}

	}
}

void LookupTable::randomise_color(int seed) {
	std::srand(seed);
	for (int i = 0; i < particle_types_count; i++) {
		int color = int(((double)rand() / (RAND_MAX)) * 0xFFFFFF);//0xA3499A;
		lookupData[i * size_one_partricle_type + 3 * particle_types_count] = *(float*)&color;
	}
}

float LookupTable::get_normalised_force_from_function(int force_function_id, float radius) {// without forcemultiplier
	//radius 0-1

	if (force_function_id == 0) {//constant
		return 1.0f;
	}
	else if (force_function_id == 1) {//linear falling
		return 1.0f - radius;
	}
	else if (force_function_id == 2) {//linear rising
		return radius;
	}
	else if (force_function_id == 3) {//sinus
		return -sin(radius * 6.283185307);
	}
	else if (force_function_id == 4) {//double linear (through + & -)
		return 2 * radius - 1.0f;
	}
	else if (force_function_id == 5) {//mountain
		float b = 10.0f;
		return (1 / (1 - 1 / (b * 0.25 + 1))) / (b * (radius - 0.5) * (radius - 0.5) + 1) - (1 / (1 - 1 / (b * 0.25 + 1)) - 1);
	}

	return 0.0f;//if all fails
}

float LookupTable::get_max_force(int stationary_particle_id, int attracted_particle_id) {
	return lookupData[stationary_particle_id * size_one_partricle_type + particle_types_count + attracted_particle_id] * max_force;
}

float LookupTable::get_influence_radius(int stationary_particle_id, int attracted_particle_id) {
	return lookupData[stationary_particle_id * size_one_partricle_type + particle_types_count * 2 + attracted_particle_id] * max_influence_radius;
}

float LookupTable::get_force_from_function(int stationary_particle_id, int attracted_particle_id, float distance) {
	distance = distance / get_influence_radius(stationary_particle_id, attracted_particle_id);//normalise distance
	if (distance > 1.0f)
		return 0.0f;
	return get_normalised_force_from_function(int(lookupData[stationary_particle_id * size_one_partricle_type + attracted_particle_id]), distance) * get_max_force(stationary_particle_id, attracted_particle_id);
}

sf::Color LookupTable::get_Color(int particle_id) {
	int color = *(int*) &lookupData[particle_id * size_one_partricle_type + 3 * particle_types_count];
	return sf::Color(color >> 16, (color >> 8) & 0x000000FF, color & 0x000000FF, 255);
}

int LookupTable::get_raw_Color_as_int(int particle_id) {
	return *(int*)&lookupData[particle_id * size_one_partricle_type + 3 * particle_types_count];// 0x 0/R/G/B
}