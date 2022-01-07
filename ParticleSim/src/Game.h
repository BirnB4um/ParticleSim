#pragma once


#include <iostream>
#include <cmath>
#include <mutex>
#include <condition_variable>
#include <string>
#include <vector>
#include <fstream>
#include <ctime>
#include <chrono>
#include <SFML/Graphics.hpp>
#include "LookupTable.h"
#include "Button.h"
#include "Slider.h"
#include "Checkbox.h"
#include "IO_DATA.h"
#include "Quadtree.h"


class Game
{

	// ===== Variables =====
private:
	//general
	IO_DATA io;
	sf::Font font;

	//simulation stuff
	sf::Event sfEvent;
	sf::Clock dtClock;
	float dt;
	int focused_particle_index;
	bool hide_hud = false;
	bool draw_quadtree = false;
	Quadtree* qt_read;
	Quadtree* qt_write;

	
	//rendering stuff
	sf::RectangleShape background;
	sf::View normal_view;
	sf::View simulation_view;
	sf::RectangleShape mouse_info_plane;
	float zoom_factor;
	float max_zoom_factor;
	float zoom_offset_x;
	float zoom_offset_y;

	//input menu
	bool show_user_input_window;
	int user_input_ID = 0;
	int user_input_value = 0;
	sf::RectangleShape user_input_plane;

	//info menu
	bool show_menu;
	sf::RectangleShape menu_plane;
	Checkbox menu_screen_wrap_checkbox;
	Slider menu_max_influence_radius_slider;
	Slider menu_max_force_slider;
	Slider menu_max_velocity_slider;
	Slider menu_friction_slider;
	Slider menu_particle_radius_slider;
	Slider menu_particle_types_slider;
	Slider menu_simulation_fps_slider;
	Button menu_random_color_button;
	Button menu_random_position_button;
	Button menu_random_behavior_button;
	Button menu_set_particle_count_button;


	//save & load menu
	bool show_save_and_load_menu_window;
	sf::RectangleShape save_and_load_plane;
	Button save_settings_button;
	Button load_settings_button;
	Button save_recording_button;
	Button load_recording_button;

	//particle
	bool set_new_particle_types_count;
	bool set_new_particle_count;
	int new_particle_types_count;
	int new_particle_count;

	//settings
	bool load_new_settings;
	char* new_loaded_settings;

	//mouse
	float mouse_grab_radius;
	float mouse_grab_radius_step;
	float max_mouse_grab_radius;
	std::vector<sf::Vector3f> grabbed_particles;//index, offsetX, offsetY
	bool is_middle_mousebutton_down;
	float middle_mouse_pressed_pos_x;
	float middle_mouse_pressed_pos_y;

	//recording
	bool recording;
	bool restart_recording;
	sf::CircleShape rec_circle;
	std::string recording_file_path;
	Button recording_framerate_button;

	//thread stuff
	bool is_changing_data;
	bool ready_for_data_change;
	



public:
	//general variables
	bool run_game;
	bool fullscreen;

	//particle data
	const int number_of_particle_data_in_4bytes = 5; //id, posX, posY, velX, velY
	float* particle_buffer;
	float* next_particle_buffer;

	//recording
	bool play_recording;
	char* loaded_position_data;
	float* loaded_current_position_data;
	char* loaded_particle_color;
	int loaded_particle_count;
	int loaded_frames;
	int simulated_frames;
	int current_frame;
	long long last_recording_time;
	long long recording_time;
	float recording_fps;
	Slider recording_frame_slider;
	Button close_recording_button;


	//for each thread
	int thread_count;
	bool* finished_threads;
	int* particles_to_simulate;
	std::mutex mutex_thread;
	std::condition_variable cv_thread;


	//simulation stuff
	bool simulation_paused;
	bool draw_hitbox;
	LookupTable lookupTable;
	float simulation_fps;
	int simulation_time_taken;//millisec
	long long simulation_time_start;
	long long time_now;
	bool do_extra_pause;
	int extra_pause_time;//milisec


	//rendering stuff
	sf::RenderWindow window;
	sf::CircleShape particle_circle;

	
	//mouse position relative to simulation space
	float mouseX = 0.0f;
	float mouseY = 0.0f;

	//mouse position relative to normal view
	float mouseX_normal = 0.0f;
	float mouseY_normal = 0.0f;

	//constants
	const int FPS = 40;
	const float SCREEN_RATIO = 16.0f / 9.0f;
	const float SCREEN_WIDTH = 2000.0f, SCREEN_HEIGHT = SCREEN_WIDTH / SCREEN_RATIO;
	const std::string TITLE = "Particle simulation";



	// ===== Functions =====
private:
	void init(unsigned int number_of_threads, unsigned int seed);
	void resize_view();
	void set_view_position(float x, float y);
	void update_menu_position();

	void updateSFMLEvents();
	void update();
	void render();

	//simulation
	void update_menu_slider_values();
	void compute_particle(int index);
	void go_through_quadtree(Quadtree& qt, float& min_x, float& max_x, float& min_y, float& max_y, int& i, float& next_vel_x, float& next_vel_y);

	//settings
	bool save_settings_to_file();
	bool load_settings_from_file();
	void set_settings();

	//recording
	void set_recording_status(bool recording);
	bool load_recording();




public:
	Game();
	Game(unsigned int number_of_threads, unsigned int seed);
	~Game();
	void run();
	void load_file_from_start(std::string file_name);
	void create_window(bool _fullscreen, int fps);

	float sqrt_fast(const float x);

	void update_particle_count_for_each_thread();

	//particles
	void randomise_particles();
	void resize_particle_count(int count);

	void set_particle_ID(float* particle_buffer, int index, int id);
	void set_particle_position(float* particle_buffer, int index, float pos_x, float pos_y);
	void set_particle_position_X(float* particle_buffer, int index, float pos_x);
	void set_particle_position_Y(float* particle_buffer, int index, float pos_y);
	void set_particle_velocity(float* particle_buffer, int index, float vel_x, float vel_y);
	void set_particle_velocity_X(float* particle_buffer, int index, float vel);
	void set_particle_velocity_Y(float* particle_buffer, int index, float vel);
	void set_particle_data(float* particle_buffer, int index, int id, float pos_x, float pos_y, float dir, float vel);

	int get_particle_ID(int index);
	float get_particle_position_X(int index);
	float get_particle_position_Y(int index);
	float get_particle_velocity_X(int index);
	float get_particle_velocity_Y(int index);

	//thread functions
	void th_manage_particle_update();
	void th_compute_particles(int thread_index);

};

