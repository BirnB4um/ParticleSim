#include "Game.h"

Game::Game()
{
	init(1, 1);
}

Game::Game(unsigned int number_of_threads, unsigned int seed)
{
	init(number_of_threads, seed);
}

Game::~Game()
{
	delete[] particle_buffer;
	delete[] next_particle_buffer;
	delete[] finished_threads;
	delete[] particles_to_simulate;
	delete[] new_loaded_settings;
	delete[] loaded_position_data;
	delete[] loaded_particle_color;
	delete[] loaded_current_position_data;
	delete qt_read;
	delete qt_write;
}

void Game::create_window(bool _fullscreen, int fps)
{
	fullscreen = _fullscreen;

	sf::ContextSettings settings;
	settings.depthBits = 24;
	settings.stencilBits = 8;
	settings.antialiasingLevel = 4;
	settings.majorVersion = 3;
	settings.minorVersion = 0;
	

	if (fullscreen) {
		window.create(sf::VideoMode::getFullscreenModes()[0], TITLE, sf::Style::Fullscreen, settings);
	}
	else {
		float w = 1000.0f;
		window.create(sf::VideoMode(w, w / SCREEN_RATIO), TITLE, sf::Style::Close | sf::Style::Resize, settings);
	}
	window.setFramerateLimit(fps);
	window.setVerticalSyncEnabled(true);

	resize_view();

}

void Game::init(unsigned int number_of_threads, unsigned int seed)
{
	//general
	run_game = true;
	sfEvent = sf::Event();
	qt_read = new Quadtree(0, 0, SCREEN_WIDTH, 0, SCREEN_HEIGHT);
	qt_write = new Quadtree(0, 0, SCREEN_WIDTH, 0, SCREEN_HEIGHT);

	//create window
	fullscreen = false;//start with windowed window
	create_window(fullscreen, FPS);
	
	//load font
	try {
		font.loadFromFile("c:\\windows\\fonts\\arial.ttf");//"c:\\windows\\fonts\\arial.ttf" //"tim_font.ttf"
	}
	catch (const std::exception& e) {
		font.loadFromFile("arial.ttf");//load font
	}

	//background plane
	background = sf::RectangleShape(sf::Vector2f(SCREEN_WIDTH, SCREEN_HEIGHT));
	background.setFillColor(sf::Color(8, 8, 8, 0));
	background.setOutlineColor(sf::Color(200, 200, 200, 255));
	background.setOutlineThickness(1);

	//mouse info plane
	mouse_info_plane.setFillColor(sf::Color(8, 8, 8, 255));
	mouse_info_plane.setOutlineColor(sf::Color(150, 150, 150, 255));
	mouse_info_plane.setOutlineThickness(3);
	
	//views
	simulation_view = sf::View(sf::Vector2f(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2), sf::Vector2f(SCREEN_WIDTH, SCREEN_HEIGHT));
	normal_view = sf::View(sf::Vector2f(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2), sf::Vector2f(SCREEN_WIDTH, SCREEN_HEIGHT));

	//lookuptable
	lookupTable.init(seed);

	//particles
	resize_particle_count(lookupTable.particle_count);

	particle_circle.setFillColor(sf::Color::White);
	particle_circle.setRadius(lookupTable.particle_radius);
	particle_circle.setOrigin(lookupTable.particle_radius, lookupTable.particle_radius);
	randomise_particles();

	set_new_particle_types_count = false;
	set_new_particle_count = false;
	new_particle_types_count = lookupTable.particle_types_count;
	new_particle_count = lookupTable.particle_count;

	//rendering stuff
	simulation_paused = true;
	draw_hitbox = false;
	zoom_factor = 1.0f;
	max_zoom_factor = 10.0f;
	zoom_offset_x = 0.0f;
	zoom_offset_y = 0.0f;

	//simulation
	simulation_fps = 60;
	simulation_time_taken = 1000;
	simulation_time_start = 0;
	do_extra_pause = false;
	extra_pause_time = 100;//0.1 sec

	//recording
	recording_fps = 30;
	play_recording = false;
	recording = false;
	restart_recording = false;
	rec_circle.setRadius(40);
	rec_circle.setOrigin(40, 40);
	rec_circle.setFillColor(sf::Color::Red);
	recording_frame_slider = Slider(0, 0, 200, 2, 15, 0.0f, 1.0f, 0.0f);
	close_recording_button = Button(0, 0, 50, 50);
	save_and_load_plane.setFillColor(sf::Color(30, 30, 30, 230));
	save_and_load_plane.setOutlineColor(sf::Color(160, 160, 160, 255));
	save_and_load_plane.setOutlineThickness(10);
	recording_framerate_button = Button(0, 0, 50, 50);


	//save & load menu
	save_settings_button = Button(0, 0, 240, 60);
	load_settings_button = Button(0, 0, 240, 60);
	save_recording_button = Button(0, 0, 240, 60);
	load_recording_button = Button(0, 0, 240, 60);




	//menus
	show_menu = false;
	show_save_and_load_menu_window = false;
	show_user_input_window = false;

	//mouse
	mouse_grab_radius = lookupTable.particle_radius;
	mouse_grab_radius_step = 10.0f;
	max_mouse_grab_radius = 400.0f;
	is_middle_mousebutton_down = false;

	simulated_frames = 0;
	current_frame = 0;
	focused_particle_index = -1;


	//input plane
	user_input_plane.setPosition(100, 100);
	user_input_plane.setSize(sf::Vector2f(SCREEN_WIDTH - 200, SCREEN_HEIGHT - 200));
	user_input_plane.setFillColor(sf::Color(0, 0, 0, 230));
	user_input_plane.setOutlineColor(sf::Color(183, 183, 183, 255));//lisa wollte 183
	user_input_plane.setOutlineThickness(6);

	//menu plane
	menu_plane.setFillColor(sf::Color(30, 30, 30, 230));
	menu_plane.setOutlineColor(sf::Color(160, 160, 160, 255));
	menu_plane.setOutlineThickness(10);

	menu_screen_wrap_checkbox = Checkbox(0, 0, 50, 50);
	menu_max_velocity_slider = Slider(0, 0, 200, 2, 0.0f, lookupTable.max_velocity, 0.25f);
	menu_friction_slider = Slider(0, 0, 200, 2, 0.8f, 1.0f, 0.5f);
	menu_max_force_slider = Slider(0, 0, 200, 2, 0.0f, 2.0f, 0.1f);
	menu_max_influence_radius_slider = Slider(0, 0, 200, 2, 25.0f, lookupTable.max_influence_radius, 0.4f);
	menu_particle_radius_slider = Slider(0, 0, 200, 2, 0.1f, lookupTable.max_particle_radius, 0.247648902f);
	menu_particle_types_slider = Slider(0, 0, 200, 2, 1.0f, lookupTable.max_particle_type_count, 0.138f);
	menu_simulation_fps_slider = Slider(0, 0, 200, 2, 1.0f, 60.0f, 1.0f);
	menu_random_color_button = Button(0, 0, 50, 50);
	menu_random_position_button = Button(0, 0, 50, 50);
	menu_random_behavior_button = Button(0, 0, 50, 50);
	menu_set_particle_count_button = Button(0, 0, 50, 50);

	//update values
	update_menu_slider_values();


	//update menu positions
	update_menu_position();


	//init threads
	thread_count = number_of_threads;
	particles_to_simulate = new int[thread_count];
	finished_threads = new bool[thread_count];
	memset(particles_to_simulate, 0, thread_count * sizeof(int));//clear data
	//set all true for first frame
	for (int i = 0; i < thread_count; i++) {
		finished_threads[i] = true;
	}
	update_particle_count_for_each_thread();

	is_changing_data = false;
	ready_for_data_change = false;
	run_game = true;
}


void Game::load_file_from_start(std::string file_name) {
	simulation_paused = true;
	show_save_and_load_menu_window = false;
	show_user_input_window = false;

	std::vector<char> input_data;
	if (io.read_from_file(file_name, input_data)) {
		char version = input_data[0];
		std::cout << int(version) << std::endl;

		if (version == 1) {
			play_recording = false;
			//load setting
			delete[] new_loaded_settings;
			new_loaded_settings = new char[input_data.size()];
			memcpy(new_loaded_settings, &input_data[0], input_data.size());
			load_new_settings = true;
		}
		else if (version == 0xA1) {
			//load recording

			//test if file is corrupted
			if (int(input_data[input_data.size() - 1]) != -1) {
				std::cout << "ERROR: File corrupted!" << std::endl;
			}

			//load radius
			int temp = 0;
			memcpy(&temp, &input_data[4], 4);
			lookupTable.particle_radius = (float(temp) / INT_MAX) * lookupTable.max_particle_radius;

			//load particle count
			memcpy(&loaded_particle_count, &input_data[8], 4);

			//load colors
			delete[] loaded_particle_color;
			loaded_particle_color = new char[loaded_particle_count * 3];
			for (int i = 0; i < loaded_particle_count; i++) {
				loaded_particle_color[i * 3 + 2] = input_data[12 + i * 4];//B
				loaded_particle_color[i * 3 + 1] = input_data[12 + i * 4 + 1];//G
				loaded_particle_color[i * 3] = input_data[12 + i * 4 + 2];//R
			}


			//load frame count
			memcpy(&loaded_frames, &input_data[input_data.size() - 5], 4);


			//load positions
			loaded_position_data = new char[loaded_frames * loaded_particle_count * 3];
			memcpy(loaded_position_data, &input_data[12 + loaded_particle_count * 4], loaded_frames * loaded_particle_count * 3);


			delete[] loaded_current_position_data;
			loaded_current_position_data = new float[loaded_particle_count * 2];


			//update slider
			recording_frame_slider.max_wert = float(loaded_frames) - 0.5f;
			recording_frame_slider.update_state(recording_frame_slider.wert);


			play_recording = true;
		}
		else {
			std::cout << "ERROR: cant open file" << std::endl;
		}
	}

}

//view and update relative positions
void Game::resize_view()
{
	float ratio = (float)window.getSize().x / float(window.getSize().y);
	if (ratio >= SCREEN_WIDTH / SCREEN_HEIGHT) {
		simulation_view.setSize(SCREEN_HEIGHT * ratio / zoom_factor, SCREEN_HEIGHT / zoom_factor);
		normal_view.setSize(SCREEN_HEIGHT * ratio, SCREEN_HEIGHT);
	}
	else {
		simulation_view.setSize(SCREEN_WIDTH / zoom_factor, SCREEN_WIDTH / (ratio * zoom_factor));
		normal_view.setSize(SCREEN_WIDTH, SCREEN_WIDTH / ratio);
	}

	//update menu position
	update_menu_position();

}

void Game::set_view_position(float x, float y) {
	simulation_view.setCenter(x, y);
}

void Game::update_menu_position() {
	//menu
	float posX = normal_view.getCenter().x - normal_view.getSize().x / 2 + menu_plane.getOutlineThickness();
	float posY = normal_view.getCenter().y - normal_view.getSize().y / 2 + menu_plane.getOutlineThickness();
	menu_plane.setPosition(sf::Vector2f(posX, posY));
	menu_plane.setSize(sf::Vector2f(520, normal_view.getSize().y - 2 * menu_plane.getOutlineThickness()));

	menu_screen_wrap_checkbox.set_position(posX + 30, posY + 30);

	menu_max_velocity_slider.set_position(posX + 30, posY + 150);
	menu_friction_slider.set_position(posX + 30, posY + 260);
	menu_max_force_slider.set_position(posX + 30, posY + 370);
	menu_max_influence_radius_slider.set_position(posX + 30, posY + 480);
	menu_particle_radius_slider.set_position(posX + 30, posY + 590);
	menu_particle_types_slider.set_position(posX + 30, posY + 700);
	menu_simulation_fps_slider.set_position(posX + 30, posY + 810);

	menu_random_color_button.set_position(posX + 30, posY + menu_plane.getSize().y - menu_random_color_button.height - 30);
	menu_random_position_button.set_position(posX + 100, posY + menu_plane.getSize().y - menu_random_color_button.height - 30);
	menu_random_behavior_button.set_position(posX + 170, posY + menu_plane.getSize().y - menu_random_color_button.height - 30);
	menu_set_particle_count_button.set_position(posX + 240, posY + menu_plane.getSize().y - menu_random_color_button.height - 30);

	//save & load menu
	posX = normal_view.getCenter().x + normal_view.getSize().x / 2 - menu_plane.getOutlineThickness() * 2 - save_and_load_plane.getSize().x;
	posY = normal_view.getCenter().y + normal_view.getSize().y / 2 - menu_plane.getOutlineThickness() * 2 - save_and_load_plane.getSize().y;
	save_and_load_plane.setSize(sf::Vector2f(300, 600));
	save_and_load_plane.setPosition(posX, posY);

	save_settings_button.set_position(posX + 30, posY + 100);
	load_settings_button.set_position(posX + 30, posY + 190);
	save_recording_button.set_position(posX + 30, posY + 380);
	load_recording_button.set_position(posX + 30, posY + 470);

	//recording
	posX = normal_view.getCenter().x - normal_view.getSize().x / 2;
	posY = normal_view.getCenter().y - normal_view.getSize().y / 2;
	close_recording_button.set_position(posX + 20, posY + 20);
	recording_framerate_button.set_position(posX + 20, posY + 90);

	float dx = 50;
	recording_frame_slider.width = normal_view.getSize().x - 2 * dx;
	recording_frame_slider.rect_shape.setSize(sf::Vector2f(recording_frame_slider.width, 2));
	recording_frame_slider.set_position(normal_view.getCenter().x - normal_view.getSize().x / 2 + dx, normal_view.getCenter().y + normal_view.getSize().y / 2 - 50);


}

//update thread zuteilung
void Game::update_particle_count_for_each_thread() {
	int particles_pro_thread = int(lookupTable.particle_count / thread_count);
	for (int i = 0; i < thread_count; i++) {
		particles_to_simulate[i] = particles_pro_thread * (i+1);

		if (i == thread_count - 1) {
			particles_to_simulate[i] = lookupTable.particle_count;
		}
	}
}


//recording
void Game::set_recording_status(bool recording) {

	if (recording) {

		/*
		 Aufbau
		
		 version 4 byte (int: 0xA1)
		 particle radius: 4 byte (int)
		 particle_count: 4 byte (int)

		 color: particle_count * 4 byte (int: 0RGB)
		 positions: particle_count * 3 byte (X & Y je 1,5 byte)

		 frames: 4 byte (int)
		 no corrupted file detector byte: 1 byte 0xFF
		 */


		recording_file_path = io.choose_save_file(2);
		if (recording_file_path == "")
			return;
		
		this->recording = recording;

		simulation_paused = true;

		//close all menus
		show_menu = false;
		show_save_and_load_menu_window = false;
		show_user_input_window = false;

		//reset frame counter
		simulated_frames = 0;

		std::vector<int> data;
		data.push_back(0xA1);//version for recording

		unsigned int radius = (lookupTable.particle_radius/lookupTable.max_particle_radius) * INT_MAX;//back: (radius / INT_MAX) * max_radius
		data.push_back(radius);
		
		data.push_back(lookupTable.particle_count);
		for (int i = 0; i < lookupTable.particle_count; i++) {
			data.push_back(lookupTable.get_raw_Color_as_int(get_particle_ID(i)));
		}
		io.save_to_file(recording_file_path, (char*)&data[0], 12 + lookupTable.particle_count * 4, false);

		simulation_paused = false;
	}
	else {
		this->recording = recording;
		char c[5];
		memcpy(&c[0], &simulated_frames, 4);
		c[4] = 0xFF;//detector byte
		io.save_to_file(recording_file_path, c, 5, true);
		recording_file_path = "";
	}
}

bool Game::load_recording() {
	std::vector<char> input_data;
	if (io.read_from_file(io.choose_open_file(2), input_data)) {

		//test if file is corrupted
		if (int(input_data[input_data.size() - 1]) != -1) {
			std::cout << "ERROR: File corrupted!" << std::endl;
			return false;
		}

		int version = 0;
		memcpy(&version, &input_data[0], 4);
		if (version == 0xA1) {
			//load radius
			int temp = 0;
			memcpy(&temp, &input_data[4], 4);
			lookupTable.particle_radius = (float(temp) / INT_MAX) * lookupTable.max_particle_radius;

			//load particle count
			memcpy(&loaded_particle_count, &input_data[8], 4);

			//load colors
			delete[] loaded_particle_color;
			loaded_particle_color = new char[loaded_particle_count * 3];
			for (int i = 0; i < loaded_particle_count; i++) {
				loaded_particle_color[i * 3 + 2] = input_data[12 + i * 4];//B
				loaded_particle_color[i * 3 + 1] = input_data[12 + i * 4 + 1];//G
				loaded_particle_color[i * 3] = input_data[12 + i * 4 + 2];//R
			}


			//load frame count
			memcpy(&loaded_frames, &input_data[input_data.size() - 5], 4);


			//load positions
			loaded_position_data = new char[loaded_frames * loaded_particle_count * 3];
			memcpy(loaded_position_data, &input_data[12 + loaded_particle_count * 4], loaded_frames * loaded_particle_count * 3);


			delete[] loaded_current_position_data;
			loaded_current_position_data = new float[loaded_particle_count * 2];


			//update slider
			recording_frame_slider.max_wert = float(loaded_frames) - 0.5f;
			recording_frame_slider.update_state(recording_frame_slider.wert);


			return true;
		}
		else {
			std::cout << "Unknown Version: " << version - 0xA0 << std::endl;
			return false;
		}

	}
	return false;

}


//setting
bool Game::save_settings_to_file() {
	/*
	-border 1 byte
	-particle count: 4 bytes
	-particle type count: 4 byte
	-particle radius 4 bytes
	-particle max vel 4 bytes
	-particle friction 4 bytes
	-particle max force 4 bytes
	-particle max influence radius 4 bytes
	-all particle data (particle_count * number_of_particle_data_in_4bytes * 4 bytes) //id, posX, posY, velX, velY
	-lookuptable byte by byte ( type_count * (3*type_count + 1) * 4 bytes )

	all: 29 + [count * number_of_particle_data_in_4bytes * 4] + [type_count * (3*type_count + 1) * 4]   bytes
	*/

	std::string file_path = io.choose_save_file(1);
	if (file_path == "")
		return false;

	char* out_data = new char[30 + lookupTable.particle_count * number_of_particle_data_in_4bytes * 4 + lookupTable.particle_types_count * (3 * lookupTable.particle_types_count + 1) * 4];
	unsigned char version = 1;

	memcpy(&out_data[0], &version, 1);
	memcpy(&out_data[1], &lookupTable.wrap_around_screen,  1);//border
	memcpy(&out_data[2], &lookupTable.particle_count,  4);//particle_count
	memcpy(&out_data[6], &lookupTable.particle_types_count, 4);//particle_type_count
	memcpy(&out_data[10], &lookupTable.particle_radius,  4);//particle_radius
	memcpy(&out_data[14], &lookupTable.max_velocity, 4);//max_velocity
	memcpy(&out_data[18], &lookupTable.friction,4);//friction
	memcpy(&out_data[22], &lookupTable.max_force, 4);//max_force
	memcpy(&out_data[26], &lookupTable.max_influence_radius, 4);//particle_influence_radius

	//all particle data
	memcpy(&out_data[30], particle_buffer, lookupTable.particle_count * number_of_particle_data_in_4bytes * 4);

	//lookuptable_data
	memcpy(&out_data[30 + lookupTable.particle_count * number_of_particle_data_in_4bytes * 4], lookupTable.lookupData, lookupTable.particle_types_count * (3 * lookupTable.particle_types_count + 1) * 4);

	io.save_to_file(file_path, out_data, 30 + lookupTable.particle_count * number_of_particle_data_in_4bytes * 4 + lookupTable.particle_types_count * (3 * lookupTable.particle_types_count + 1) * 4, false);
	delete[] out_data;
	return true;
}

bool Game::load_settings_from_file() {
	std::vector<char> input_data;
	std::string file_name = io.choose_open_file(1);
	if (file_name == "")
		return false;

	if (io.read_from_file(file_name, input_data)) {
		char version = input_data[0];
		if (version == 1) {
			delete[] new_loaded_settings;
			new_loaded_settings = new char[input_data.size()];
			memcpy(new_loaded_settings, &input_data[0], input_data.size());
			load_new_settings = true;
			return true;
		}
		else {
			std::cout << "Unknown Version: " << unsigned int(version) << std::endl;
		}
	}
	return false;
}

void Game::set_settings() {

	memcpy(&lookupTable.wrap_around_screen, &new_loaded_settings[1], 1);//border
	memcpy(&lookupTable.particle_count, &new_loaded_settings[2], 4);//particle_count
	memcpy(&lookupTable.particle_types_count, &new_loaded_settings[6], 4);//particle_type_count
	memcpy(&lookupTable.particle_radius, &new_loaded_settings[10], 4);//particle_radius
	memcpy(&lookupTable.max_velocity, &new_loaded_settings[14], 4);//max_velocity
	memcpy(&lookupTable.friction, &new_loaded_settings[18], 4);//friction
	memcpy(&lookupTable.max_force, &new_loaded_settings[22], 4);//max_force
	memcpy(&lookupTable.max_influence_radius, &new_loaded_settings[26], 4);//particle_influence_radius

	//update sliders/checkboxes
	menu_screen_wrap_checkbox.update_state(!lookupTable.wrap_around_screen);//border
	menu_particle_types_slider.update_state(lookupTable.particle_types_count);//types count
	menu_particle_radius_slider.update_state(lookupTable.particle_radius);//radius
	menu_max_velocity_slider.update_state(lookupTable.max_velocity);//max velocity
	menu_friction_slider.update_state(lookupTable.friction);//friction
	menu_max_force_slider.update_state(lookupTable.max_force);//max force
	menu_max_influence_radius_slider.update_state(lookupTable.max_influence_radius);//influence radius


	//resize particle count
	delete[] particle_buffer;
	delete[] next_particle_buffer;
	particle_buffer = new float[lookupTable.particle_count * number_of_particle_data_in_4bytes];
	next_particle_buffer = new float[lookupTable.particle_count * number_of_particle_data_in_4bytes];
	//set memory of buffers
	memcpy(particle_buffer, &new_loaded_settings[30], lookupTable.particle_count * number_of_particle_data_in_4bytes * 4);
	memcpy(next_particle_buffer, &new_loaded_settings[30], lookupTable.particle_count * number_of_particle_data_in_4bytes * 4);


	//lookuptable_data
	delete[] lookupTable.lookupData;
	lookupTable.size_one_partricle_type = (lookupTable.particle_types_count * 3 + 1);
	lookupTable.lookupData_size = lookupTable.particle_types_count * lookupTable.size_one_partricle_type;
	lookupTable.lookupData = new float[lookupTable.lookupData_size];
	memcpy(lookupTable.lookupData, &new_loaded_settings[30 + lookupTable.particle_count * number_of_particle_data_in_4bytes * 4], lookupTable.lookupData_size * 4);

}



//particle funcs
void Game::randomise_particles() {
	int id;
	float posX, posY, velX, velY;
	for (int i = 0; i < lookupTable.particle_count; i++) {
		id = std::rand() % lookupTable.particle_types_count;
		posX = ((double)rand() / (RAND_MAX)) * SCREEN_WIDTH;
		posY = ((double)rand() / (RAND_MAX)) * SCREEN_HEIGHT;
		velX = ((double)rand() / (RAND_MAX)) - 0.5;
		velY = ((double)rand() / (RAND_MAX)) - 0.5;
		set_particle_data(particle_buffer, i, id, posX, posY, velX, velY);
		set_particle_data(next_particle_buffer, i, id, posX, posY, velX, velY);
	}
}

void Game::resize_particle_count(int count) {
	delete[] particle_buffer;
	delete[] next_particle_buffer;

	lookupTable.particle_count = count;

	//set two buffers for particle data
	particle_buffer = new float[lookupTable.particle_count * number_of_particle_data_in_4bytes];
	next_particle_buffer = new float[lookupTable.particle_count * number_of_particle_data_in_4bytes];

	//clear memory of buffers
	memset(particle_buffer, 0, lookupTable.particle_count * number_of_particle_data_in_4bytes * sizeof(float));
	memset(next_particle_buffer, 0, lookupTable.particle_count * number_of_particle_data_in_4bytes * sizeof(float));

	randomise_particles();
}


//simulation funcs
void Game::update_menu_slider_values() {
	lookupTable.max_velocity = menu_max_velocity_slider.get_value();
	lookupTable.friction = menu_friction_slider.get_value();
	lookupTable.max_force = menu_max_force_slider.get_value();
	lookupTable.max_influence_radius = menu_max_influence_radius_slider.get_value();
	lookupTable.particle_radius = menu_particle_radius_slider.get_value();
	new_particle_types_count = int(menu_particle_types_slider.get_value());
	simulation_fps = int(menu_simulation_fps_slider.get_value());
}

float Game::sqrt_fast(const float x)
{
	union
	{
		int i;
		float x;
	} u;
	u.x = x;
	u.i = (1 << 29) + (u.i >> 1) - (1 << 22);

	// Two Babylonian Steps (simplified from:)
	// u.x = 0.5f * (u.x + x/u.x);
	// u.x = 0.5f * (u.x + x/u.x);
	u.x = u.x + x / u.x;
	u.x = 0.25f * u.x + x / u.x;

	return u.x;
}

void Game::compute_particle(int index) {

}

//thread functions
void Game::th_manage_particle_update() {
	while (run_game) {

		if (play_recording) {
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
			continue;
		}
		
		if (simulation_paused) {
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}

		//check if all threads finished
		bool all_finished = true;
		for (int i = 0; i < thread_count; i++) {
			if (!finished_threads[i])
				all_finished = false;
		}
		if (all_finished) {

			if (!simulation_paused) {

				//create quadtree
				for (int i = 0; i < lookupTable.particle_count; i++) {
					qt_write->insert(i, get_particle_position_X(i), get_particle_position_Y(i));
				}

				//switch particle buffers
				float* temp_ptr = particle_buffer;
				particle_buffer = next_particle_buffer;
				next_particle_buffer = temp_ptr;

				//switch quadtree pointers
				Quadtree* t_ptr = qt_read;
				qt_read = qt_write;
				qt_write = t_ptr;

				qt_write->clear();
				//qt_write = new Quadtree(0, 0, SCREEN_WIDTH, 0, SCREEN_HEIGHT);
			}

			if (!recording) {

				if (set_new_particle_count || set_new_particle_types_count || load_new_settings) {
					is_changing_data = true;

					qt_read->clear();
					qt_write->clear();

					while (!ready_for_data_change) {
						std::this_thread::sleep_for(std::chrono::milliseconds(50));//20 checks per sec
					}
				}

				//set new  particle count
				if (set_new_particle_count) {
					set_new_particle_count = false;
					resize_particle_count(new_particle_count);
					update_particle_count_for_each_thread();

					
				}
				//set new particle types count
				if (set_new_particle_types_count) {
					set_new_particle_types_count = false;
					lookupTable.resize_table(rand(), new_particle_types_count);
					lookupTable.randomise_color(rand());
					for (int i = 0; i < lookupTable.particle_count; i++) {
						int id = std::rand() % lookupTable.particle_types_count;
						set_particle_ID(particle_buffer, i, id);
						set_particle_ID(next_particle_buffer, i, id);
					}
				}
				//load new settings
				if (load_new_settings) {
					load_new_settings = false;
					set_settings();
					new_particle_count = lookupTable.particle_count;
					update_particle_count_for_each_thread();
				}

				is_changing_data = false;
			}

			if (!simulation_paused) {
				//end timer
				simulation_time_taken = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count() - simulation_time_start;

				//start timer
				simulation_time_start = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

				//wait if to fast
				if (simulation_time_taken < 1000 / simulation_fps && int(1000 / simulation_fps) - simulation_time_taken < 1000) {
					std::this_thread::sleep_for(std::chrono::milliseconds(int(1000 / simulation_fps) - simulation_time_taken));
				}

				//extra pause
				if (do_extra_pause) {
					std::this_thread::sleep_for(std::chrono::milliseconds(extra_pause_time));
				}


				//notify threads for next frame
				memset(finished_threads, 0, thread_count * sizeof(bool));//clear data
				cv_thread.notify_all();


				//save recording frame to file
				if (recording) {
					char* data = new char[lookupTable.particle_count * 3];
					unsigned int temp;
					//unsigned int x, y;
					for (int i = 0; i < lookupTable.particle_count; i++) {
						temp = ((int((get_particle_position_X(i) / SCREEN_WIDTH) * 4095) & 0xFFF) << 12) + (int((get_particle_position_Y(i) / SCREEN_HEIGHT) * 4095) & 0xFFF);//x + y
						memcpy(&data[3 * i], &temp, 3);
					}
					//save to file
					io.save_to_file(recording_file_path, data, lookupTable.particle_count * 3, true);
					simulated_frames++;
					delete[] data;
				}
			}
		}
		else {
			std::this_thread::sleep_for(std::chrono::milliseconds(10));//sleeping to not overload this thread when doing nothing
		}

	}
}


void Game::go_through_quadtree(Quadtree &qt, float &min_x, float &max_x, float &min_y, float &max_y,
	int &i, float &next_vel_x, float &next_vel_y) {

	if (qt.contains_rect(min_x, max_x, min_y, max_y)) {

		//compute particles
		float next_pos_x = get_particle_position_X(i);
		float next_pos_y = get_particle_position_Y(i);
		float pos_differenze_X = 0;
		float pos_differenze_Y = 0;
		float distance = 0;
		float alpha = 0;
		float force = 0;

		for (int n : qt.index_list) {
			if (i != n) {//if these particles are not the same
				pos_differenze_X = get_particle_position_X(n) - next_pos_x;
				pos_differenze_Y = get_particle_position_Y(n) - next_pos_y;

				//if in reach-circle
				distance = sqrt_fast(pos_differenze_X * pos_differenze_X + pos_differenze_Y * pos_differenze_Y);
				if (distance < lookupTable.get_influence_radius(get_particle_ID(n), get_particle_ID(i))) {
					//get force and add to total velocity
					alpha = atan2f(pos_differenze_Y, pos_differenze_X);
					force = distance < lookupTable.particle_radius * 2 ? -1 : lookupTable.get_force_from_function(get_particle_ID(n), get_particle_ID(i), distance);
					next_vel_x += force * cos(alpha);
					next_vel_y += force * sin(alpha);
				}
			}
		}

		if (qt.subnodes[0] != nullptr) {
			go_through_quadtree( *(qt.subnodes[0]), min_x, max_x, min_y, max_y, i, next_vel_x, next_vel_y);
			go_through_quadtree( *(qt.subnodes[1]), min_x, max_x, min_y, max_y, i, next_vel_x, next_vel_y);
			go_through_quadtree( *(qt.subnodes[2]), min_x, max_x, min_y, max_y, i, next_vel_x, next_vel_y);
			go_through_quadtree( *(qt.subnodes[3]), min_x, max_x, min_y, max_y, i, next_vel_x, next_vel_y);
		}
	}
}


void Game::th_compute_particles(int thread_index) {

	float next_pos_x = 0.0f;
	float next_pos_y = 0.0f;
	float next_vel_x = 0.0f;
	float next_vel_y = 0.0f;

	float vel_length = 0.0f;


	int first_particle = 0;
	int last_particle = 0;

	float influence_rect_min_x, influence_rect_max_x;
	float influence_rect_min_y, influence_rect_max_y;

	while (true) {

		//lock thread until ready
		std::unique_lock<std::mutex> lk(mutex_thread);
		cv_thread.wait(lk);
		lk.unlock();


		if (!run_game) {
			finished_threads[thread_index] = true;
			return;
		}



		first_particle = thread_index == 0 ? 0 : particles_to_simulate[thread_index - 1];
		last_particle = particles_to_simulate[thread_index];


		for (int i = first_particle; i < last_particle; i++) {//for every particle

			next_pos_x = get_particle_position_X(i);
			next_pos_y = get_particle_position_Y(i);

			next_vel_x = get_particle_velocity_X(i);
			next_vel_y = get_particle_velocity_Y(i);


			influence_rect_min_x = get_particle_position_X(i) - lookupTable.max_influence_radius;
			influence_rect_max_x = get_particle_position_X(i) + lookupTable.max_influence_radius;
			influence_rect_min_y = get_particle_position_Y(i) - lookupTable.max_influence_radius;
			influence_rect_max_y = get_particle_position_Y(i) + lookupTable.max_influence_radius;

			//go through quadtree
			go_through_quadtree(*qt_read, influence_rect_min_x, influence_rect_max_x, influence_rect_min_y, influence_rect_max_y, i, next_vel_x, next_vel_y);


			//for (int n = 0; n < lookupTable.particle_count; n++) {
			//	if (i != n) {//if these particles are not the same
			//		float pos_differenze_X = get_particle_position_X(n) - next_pos_x;
			//		float pos_differenze_Y = get_particle_position_Y(n) - next_pos_y;
			//		//if in reach-circle
			//		float distance = sqrt_fast(pos_differenze_X * pos_differenze_X + pos_differenze_Y * pos_differenze_Y);
			//		if (distance < lookupTable.get_influence_radius(get_particle_ID(n), get_particle_ID(i))) {
			//			//get force and add to total velocity
			//			float alpha = atan2f(pos_differenze_Y, pos_differenze_X);
			//			float force = distance < lookupTable.particle_radius * 2 ? -1 : lookupTable.get_force_from_function(get_particle_ID(n), get_particle_ID(i), distance);
			//			next_vel_x += force * cos(alpha);
			//			next_vel_y += force * sin(alpha);
			//		}
			//	}
			//}


			//check if next_vel is greater than max_vel
			vel_length = sqrt_fast(next_vel_x * next_vel_x + next_vel_y * next_vel_y);
			if (vel_length > lookupTable.max_velocity) {
				next_vel_x *= lookupTable.max_velocity / vel_length;
				next_vel_y *= lookupTable.max_velocity / vel_length;
			}
			next_vel_x *= lookupTable.friction;
			next_vel_y *= lookupTable.friction;

			next_pos_x += next_vel_x;
			next_pos_y += next_vel_y;

			//check if position over bounds
			if (lookupTable.wrap_around_screen) {
				if (next_pos_x < 0.0f) {
					next_pos_x = SCREEN_WIDTH;
				}
				else if (next_pos_x > SCREEN_WIDTH) {
					next_pos_x = 0.0f;
				}

				if (next_pos_y < 0.0f) {
					next_pos_y = SCREEN_HEIGHT;
				}
				else if (next_pos_y > SCREEN_HEIGHT) {
					next_pos_y = 0.0f;
				}
			}
			else {

				if (next_pos_x < 0.0f) {
					next_pos_x = 0.0f;
					next_vel_x *= -1;
				}
				else if (next_pos_x > SCREEN_WIDTH) {
					next_pos_x = SCREEN_WIDTH;
					next_vel_x *= -1;
				}

				if (next_pos_y < 0.0f) {
					next_pos_y = 0.0f;
					next_vel_y *= -1;
				}
				else if (next_pos_y > SCREEN_HEIGHT) {
					next_pos_y = SCREEN_HEIGHT;
					next_vel_y *= -1;
				}
			}

			set_particle_position(next_particle_buffer, i, next_pos_x, next_pos_y);
			set_particle_velocity(next_particle_buffer, i, next_vel_x, next_vel_y);


		}


		//tells game that it finished
		finished_threads[thread_index] = true;

		


	}
}


//=======================================

//main loop
void Game::run()
{

	int fps_update_time = 1;//seconds
	int last_time = 0;
	while (window.isOpen())
	{

		dt = dtClock.restart().asSeconds();
		if (time(NULL) > last_time + fps_update_time) {
			last_time = time(NULL);

			if (simulation_time_taken > 1000) {//less than 1 fps
				window.setTitle(TITLE + "   SPF: " + std::to_string(simulation_time_taken / 1000.0f) + " sec.");
			}
			else {
				window.setTitle(TITLE + "   FPS: " + std::to_string(int(1000.0f / simulation_time_taken) > simulation_fps ? int(simulation_fps) : int(1000.0f / simulation_time_taken)));
			}
		}

		//wait for manager thread to change data
		if (is_changing_data) {
			ready_for_data_change = true;
			while (is_changing_data) {
				std::this_thread::sleep_for(std::chrono::milliseconds(50));
			}
			ready_for_data_change = false;
		}

		updateSFMLEvents();
		update();
		render();
	}

}

//update events
void Game::updateSFMLEvents()
{

	while (window.pollEvent(sfEvent))
	{
		//close App
		if (sfEvent.type == sf::Event::Closed) {
			run_game = false;
			while (true) {
				bool all_finished = true;
				for (int i = 0; i < thread_count; i++) {
					if (!finished_threads[i])
						all_finished = false;
				}
				if (all_finished) {
					window.close();
					return;
				}
				cv_thread.notify_all();
				std::this_thread::sleep_for(std::chrono::milliseconds(100));
			}
		}

		
		////Keyboard////
		//release
		if (sfEvent.type == sf::Event::KeyReleased) {

			//fullscreen toggle
			if (sfEvent.key.code == sf::Keyboard::F11 || (fullscreen && sfEvent.key.code == sf::Keyboard::Escape)) {
				create_window(!fullscreen, FPS);
			}
			
			if (sfEvent.key.code == sf::Keyboard::F1) {
				hide_hud = !hide_hud;
			}

			//user input plane
			if (show_user_input_window) {
				if (sfEvent.key.code == sf::Keyboard::Escape) {
					show_user_input_window = false;
					user_input_ID = 0;
					return;
				}
				if (sfEvent.key.code == sf::Keyboard::Num0) {
					user_input_value = user_input_value * 10;
				}
				else if (sfEvent.key.code == sf::Keyboard::Num1) {
					user_input_value = user_input_value * 10 + 1;
				}
				else if (sfEvent.key.code == sf::Keyboard::Num2) {
					user_input_value = user_input_value * 10 + 2;
				}
				else if (sfEvent.key.code == sf::Keyboard::Num3) {
					user_input_value = user_input_value * 10 + 3;
				}
				else if (sfEvent.key.code == sf::Keyboard::Num4) {
					user_input_value = user_input_value * 10 + 4;
				}
				else if (sfEvent.key.code == sf::Keyboard::Num5) {
					user_input_value = user_input_value * 10 + 5;
				}
				else if (sfEvent.key.code == sf::Keyboard::Num6) {
					user_input_value = user_input_value * 10 + 6;
				}
				else if (sfEvent.key.code == sf::Keyboard::Num7) {
					user_input_value = user_input_value * 10 + 7;
				}
				else if (sfEvent.key.code == sf::Keyboard::Num8) {
					user_input_value = user_input_value * 10 + 8;
				}
				else if (sfEvent.key.code == sf::Keyboard::Num9) {
					user_input_value = user_input_value * 10 + 9;
				}
				else if (sfEvent.key.code == sf::Keyboard::Backspace) {
					user_input_value = int(user_input_value / 10);
				}
				else if (sfEvent.key.code == sf::Keyboard::Enter) {
					if (user_input_ID == 1) {//change particle count
						set_new_particle_count = true;
						new_particle_count = user_input_value < 1 ? 1 : user_input_value > lookupTable.max_particle_count ? lookupTable.max_particle_count : user_input_value;
					}else if (user_input_ID == 2) {//change rec framerate
						recording_fps = user_input_value = user_input_value < 1 ? 1 : user_input_value > 60 ? 60 : user_input_value;
					}
					show_user_input_window = false;
					user_input_ID = 0;
				}

				if (user_input_ID == 1) {//change particle count
					user_input_value = user_input_value < 0 ? 0 : user_input_value > lookupTable.max_particle_count ? lookupTable.max_particle_count : user_input_value;
				}
				else if (user_input_ID == 2) {//change rec framerate
					user_input_value = user_input_value < 0 ? 0 : user_input_value > 60 ? 60 : user_input_value;
				}

			}

			if (!play_recording) {

				if (!recording) {
					//randomise particle color
					if (sfEvent.key.code == sf::Keyboard::C) {
						lookupTable.randomise_color(rand());
					}
					//menu toggle
					else if (sfEvent.key.code == sf::Keyboard::LAlt) {
						show_menu = !show_menu;
					}
				}
				//stop recording
				else if (sfEvent.key.code == sf::Keyboard::Enter) {
					set_recording_status(false);
				}

				//draw hitbox toggle
				if (sfEvent.key.code == sf::Keyboard::H) {
					draw_hitbox = !draw_hitbox;
				}

				//randomise particle position
				else if (sfEvent.key.code == sf::Keyboard::P) {
					randomise_particles();
				}

				//randomise particle behavior
				else if (sfEvent.key.code == sf::Keyboard::B) {
					lookupTable.randomise_table(rand());
				}


				//toggle border
				else if (sfEvent.key.code == sf::Keyboard::W) {
					lookupTable.wrap_around_screen = !lookupTable.wrap_around_screen;
					menu_screen_wrap_checkbox.is_checked = lookupTable.wrap_around_screen;
					menu_screen_wrap_checkbox.update_state(!lookupTable.wrap_around_screen);
				}

				//all particles on one spot
				else if (sfEvent.key.code == sf::Keyboard::Tab) {
					for (int i = 0; i < lookupTable.particle_count; i++) {
						set_particle_position(particle_buffer, i, mouseX + (((double)rand() / (RAND_MAX)) * 2 - 1) * mouse_grab_radius, mouseY + (((double)rand() / (RAND_MAX)) * 2 - 1) * mouse_grab_radius);
						set_particle_position(next_particle_buffer, i, mouseX + (((double)rand() / (RAND_MAX)) * 2 - 1) * mouse_grab_radius, mouseY + (((double)rand() / (RAND_MAX)) * 2 - 1) * mouse_grab_radius);
					}
				}




				if (!recording) {
					//toggle save/load menu
					if (sfEvent.key.code == sf::Keyboard::L) {
						show_save_and_load_menu_window = !show_save_and_load_menu_window;
					}
					//toggle "enter count" window
					else if (sfEvent.key.code == sf::Keyboard::N) {
						show_user_input_window = !show_user_input_window;
						new_particle_count = lookupTable.particle_count;
						user_input_value = new_particle_count;
						user_input_ID = 1;
					}//save settings
					else if (sfEvent.key.code == sf::Keyboard::S && sf::Keyboard::isKeyPressed(sf::Keyboard::LControl)) {
						//save_settings_to_file();
						show_save_and_load_menu_window = !show_save_and_load_menu_window;
					}
				}
			} else {
				if (sfEvent.key.code == sf::Keyboard::F) {//change framerate
					user_input_ID = 2;
					show_user_input_window = true;
					user_input_value = recording_fps;
				}
			}

			//pause simulation toggle
			if (sfEvent.key.code == sf::Keyboard::Space) {
				simulation_paused = !simulation_paused;
			}
			else if (sfEvent.key.code == sf::Keyboard::D) {
				draw_quadtree = !draw_quadtree;
			}



		}



		////Mouse////
		//wheel
		if (sfEvent.type == sf::Event::MouseWheelMoved) {
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::LControl)) {//change zoom
				zoom_factor += sfEvent.mouseWheel.delta * 0.25f;
				zoom_factor = zoom_factor < 0.5f ? 0.5f : zoom_factor > max_zoom_factor ? max_zoom_factor : zoom_factor;
				resize_view();
			}
			else if(!play_recording){//change grab particle size
				mouse_grab_radius += sfEvent.mouseWheel.delta * mouse_grab_radius_step;
				
				//keep radius in range
				mouse_grab_radius = mouse_grab_radius > max_mouse_grab_radius ? max_mouse_grab_radius : mouse_grab_radius < lookupTable.particle_radius ? lookupTable.particle_radius : mouse_grab_radius;
			}
		}

		//Button released
		if (sfEvent.type == sf::Event::MouseButtonReleased) {
			
			//release middle mouse button
			if (sfEvent.mouseButton.button == sf::Mouse::Button::Middle){
				is_middle_mousebutton_down = false;
			}

			//remove all grabbed particles
			grabbed_particles.clear();

			if (!play_recording) {
				//update checkboxes/buttons/sliders
				if (show_menu) {
					//checkbox
					if (menu_screen_wrap_checkbox.update_mouse_event(mouseX_normal, mouseY_normal, false)) {
						lookupTable.wrap_around_screen = !menu_screen_wrap_checkbox.is_checked;
					}

					//button
					if (menu_random_color_button.update_mouse_event(mouseX_normal, mouseY_normal, false)) {
						int r = rand();
						lookupTable.randomise_color(r);
					}
					if (menu_random_position_button.update_mouse_event(mouseX_normal, mouseY_normal, false)) {
						randomise_particles();
					}
					if (menu_random_behavior_button.update_mouse_event(mouseX_normal, mouseY_normal, false)) {
						lookupTable.randomise_table(rand());
					}
					if (menu_set_particle_count_button.update_mouse_event(mouseX_normal, mouseY_normal, false)) {
						show_user_input_window = true;
						user_input_ID = 1;
						user_input_value = lookupTable.particle_count;
					}

					//slider
					menu_max_velocity_slider.update_mouse_event(mouseX_normal, mouseY_normal, false);
					menu_friction_slider.update_mouse_event(mouseX_normal, mouseY_normal, false);
					menu_max_force_slider.update_mouse_event(mouseX_normal, mouseY_normal, false);
					menu_max_influence_radius_slider.update_mouse_event(mouseX_normal, mouseY_normal, false);
					menu_particle_radius_slider.update_mouse_event(mouseX_normal, mouseY_normal, false);
					menu_simulation_fps_slider.update_mouse_event(mouseX_normal, mouseY_normal, false);

					if (menu_particle_types_slider.is_pressed)
						set_new_particle_types_count = true;
					menu_particle_types_slider.update_mouse_event(mouseX_normal, mouseY_normal, false);
				}
				if (show_save_and_load_menu_window) {
					//buttons

					//settings
					if (save_settings_button.update_mouse_event(mouseX_normal, mouseY_normal, false)) {//save
						simulation_paused = true;
						save_settings_to_file();
						while (window.pollEvent(sfEvent));//clear events -> no direct enter press that stop
						return;
					}
					if (load_settings_button.update_mouse_event(mouseX_normal, mouseY_normal, false)) {//load
						simulation_paused = true;
						if (load_settings_from_file()) {
							show_menu = false;
							show_save_and_load_menu_window = false;
						}
						while (window.pollEvent(sfEvent));//clear events -> no direct enter press that stop
						return;
					}

					//recording
					if (save_recording_button.update_mouse_event(mouseX_normal, mouseY_normal, false)) {//save
						simulation_paused = true;
						set_recording_status(true);
						while(window.pollEvent(sfEvent));//clear events -> no direct enter press that stop
						return;
					}
					if (load_recording_button.update_mouse_event(mouseX_normal, mouseY_normal, false)) {//load
						simulation_paused = true;
						if (load_recording()) {
							show_menu = false;
							show_save_and_load_menu_window = false;
							show_user_input_window = false;
							play_recording = true;
						}
						while (window.pollEvent(sfEvent));//clear events -> no direct enter press that stop
						return;
					}
				}
			}
			else {
				//slider
				recording_frame_slider.update_mouse_event(mouseX_normal, mouseY_normal, false);

				//button
				if (close_recording_button.update_mouse_event(mouseX_normal, mouseY_normal, false)) {
					play_recording = false;
					simulation_paused = true;
				}
				if (recording_framerate_button.update_mouse_event(mouseX_normal, mouseY_normal, false)) {
					user_input_ID = 2;
					user_input_value = recording_fps;
					show_user_input_window = true;
				}
			}
		}

		//Button pressed
		if (sfEvent.type == sf::Event::MouseButtonPressed) {

			//update checkboxes/buttons/sliders
			if (show_menu) {
				//checkbox
				menu_screen_wrap_checkbox.update_mouse_event(mouseX_normal, mouseY_normal, true);

				//button
				menu_random_color_button.update_mouse_event(mouseX_normal, mouseY_normal, true);
				menu_random_position_button.update_mouse_event(mouseX_normal, mouseY_normal, true);
				menu_random_behavior_button.update_mouse_event(mouseX_normal, mouseY_normal, true);
				menu_set_particle_count_button.update_mouse_event(mouseX_normal, mouseY_normal, true);

				//slider
				menu_max_velocity_slider.update_mouse_event(mouseX_normal, mouseY_normal, true);
				menu_friction_slider.update_mouse_event(mouseX_normal, mouseY_normal, true);
				menu_max_force_slider.update_mouse_event(mouseX_normal, mouseY_normal, true);
				menu_max_influence_radius_slider.update_mouse_event(mouseX_normal, mouseY_normal, true);
				menu_particle_radius_slider.update_mouse_event(mouseX_normal, mouseY_normal, true);
				menu_particle_types_slider.update_mouse_event(mouseX_normal, mouseY_normal, true);
				menu_simulation_fps_slider.update_mouse_event(mouseX_normal, mouseY_normal, true);
			}
			if (show_save_and_load_menu_window) {
				//buttons
				save_settings_button.update_mouse_event(mouseX_normal, mouseY_normal, true);
				load_settings_button.update_mouse_event(mouseX_normal, mouseY_normal, true);
				save_recording_button.update_mouse_event(mouseX_normal, mouseY_normal, true);
				load_recording_button.update_mouse_event(mouseX_normal, mouseY_normal, true);
			}
			if (play_recording) {
				//slider
				if (recording_frame_slider.update_mouse_event(mouseX_normal, mouseY_normal, true)) {
					restart_recording = false;
				}


				//button 
				close_recording_button.update_mouse_event(mouseX_normal, mouseY_normal, true);
				recording_framerate_button.update_mouse_event(mouseX_normal, mouseY_normal, true);
			}

			//if mouse not over menu or save/load mennu
			if (!(show_menu && mouseX_normal > menu_plane.getPosition().x && mouseX_normal < menu_plane.getPosition().x + menu_plane.getSize().x &&
				mouseY_normal > menu_plane.getPosition().y && mouseY_normal < menu_plane.getPosition().y + menu_plane.getSize().y) &&
				!(show_save_and_load_menu_window && mouseX_normal > save_and_load_plane.getPosition().x && mouseX_normal < save_and_load_plane.getPosition().x + save_and_load_plane.getSize().x &&
					mouseY_normal > save_and_load_plane.getPosition().y && mouseY_normal < save_and_load_plane.getPosition().y + save_and_load_plane.getSize().y)) {

				//zoom offset
				if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Middle) && focused_particle_index == -1) {
					is_middle_mousebutton_down = true;
					middle_mouse_pressed_pos_x = mouseX;
					middle_mouse_pressed_pos_y = mouseY;
					return;
				}


				//grab particle
				bool leftclick_on_particle = false;
				bool rigth_mouse_pressed = sf::Mouse::isButtonPressed(sf::Mouse::Button::Right);
				bool left_mouse_pressed = sf::Mouse::isButtonPressed(sf::Mouse::Button::Left);
				if (!play_recording) {
					for (int i = 0; i < lookupTable.particle_count; i++) {
						if (left_mouse_pressed) {
							float dist = sqrt((mouseX - get_particle_position_X(i)) * (mouseX - get_particle_position_X(i)) + (mouseY - get_particle_position_Y(i)) * (mouseY - get_particle_position_Y(i)));
							if (dist < mouse_grab_radius) {
								//add to grab-list
								grabbed_particles.push_back(sf::Vector3f(get_particle_position_X(i) - mouseX, get_particle_position_Y(i) - mouseY, i));
							}
						}
						else if (rigth_mouse_pressed) {
							if (sqrt((mouseX - get_particle_position_X(i)) * (mouseX - get_particle_position_X(i)) + (mouseY - get_particle_position_Y(i)) * (mouseY - get_particle_position_Y(i))) < lookupTable.particle_radius) {
								focused_particle_index = i;
								leftclick_on_particle = true;
								break;
							}
						}
					}
				}
				//set focus
				if (rigth_mouse_pressed) {
					if (leftclick_on_particle) {
						zoom_offset_x = 0.0f;
						zoom_offset_y = 0.0f;
						set_view_position(get_particle_position_X(focused_particle_index), get_particle_position_Y(focused_particle_index));
					}
					else {
						focused_particle_index = -1;
						zoom_offset_x = 0.0f;
						zoom_offset_y = 0.0f;
						zoom_factor = 1.0f;
						resize_view();
						set_view_position(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2);
					}
				}


			}

		}


		//window resized
		if (sfEvent.type == sf::Event::Resized)
			resize_view();

	}

}

//update data
void Game::update()
{

	if (play_recording) {//replay recording
		//buttons
		close_recording_button.update(mouseX_normal, mouseY_normal);
		recording_framerate_button.update(mouseX_normal, mouseY_normal);

		//slider
		recording_frame_slider.update(mouseX_normal, mouseY_normal);
		current_frame = int(recording_frame_slider.get_value());


		//simualtion step
		recording_time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
		if (!simulation_paused && last_recording_time < recording_time) {
			last_recording_time = recording_time + int(1000.0f / recording_fps);
			
			//restart simulation
			if (restart_recording) {
				restart_recording = false;
				recording_frame_slider.update_state(0);
			}
			else {
				//increase one frame
				if (!recording_frame_slider.is_pressed)
					recording_frame_slider.update_state(recording_frame_slider.get_value() + 1);
			}

			//check if end of recording
			if (recording_frame_slider.get_value() == recording_frame_slider.max_wert) {
				simulation_paused = true;
				restart_recording = true;
			}


			
		}



	}
	else {//live simulation

		//update checkboxes/buttons/sliders
		if (show_menu) {
			//checkboxes
			menu_screen_wrap_checkbox.update(mouseX_normal, mouseY_normal);

			//buttons
			menu_random_color_button.update(mouseX_normal, mouseY_normal);
			menu_random_position_button.update(mouseX_normal, mouseY_normal);
			menu_random_behavior_button.update(mouseX_normal, mouseY_normal);
			menu_set_particle_count_button.update(mouseX_normal, mouseY_normal);

			//slider
			menu_max_velocity_slider.update(mouseX_normal, mouseY_normal);
			menu_friction_slider.update(mouseX_normal, mouseY_normal);
			menu_max_force_slider.update(mouseX_normal, mouseY_normal);
			menu_max_influence_radius_slider.update(mouseX_normal, mouseY_normal);
			menu_particle_radius_slider.update(mouseX_normal, mouseY_normal);
			menu_particle_types_slider.update(mouseX_normal, mouseY_normal);
			menu_simulation_fps_slider.update(mouseX_normal, mouseY_normal);
		}
		if (show_save_and_load_menu_window) {
			save_settings_button.update(mouseX_normal, mouseY_normal);
			load_settings_button.update(mouseX_normal, mouseY_normal);
			save_recording_button.update(mouseX_normal, mouseY_normal);
			load_recording_button.update(mouseX_normal, mouseY_normal);
		}

		update_menu_slider_values();

		//set grabbed particles to mouse position
		for (const sf::Vector3f& grabbed_particle : grabbed_particles) {
			set_particle_position_X(particle_buffer, grabbed_particle.z, grabbed_particle.x + mouseX);
			set_particle_position_Y(particle_buffer, grabbed_particle.z, grabbed_particle.y + mouseY);
			set_particle_position_X(next_particle_buffer, grabbed_particle.z, grabbed_particle.x + mouseX);
			set_particle_position_Y(next_particle_buffer, grabbed_particle.z, grabbed_particle.y + mouseY);
		}


		//set view to focused particle
		if (focused_particle_index > -1)
			set_view_position(get_particle_position_X(focused_particle_index), get_particle_position_Y(focused_particle_index));

	}



	//update mouse position relative to simulation spaces
	mouseX = ((float)sf::Mouse::getPosition(window).x / window.getSize().x) * simulation_view.getSize().x + (simulation_view.getCenter().x - simulation_view.getSize().x / 2);
	mouseY = ((float)sf::Mouse::getPosition(window).y / window.getSize().y) * simulation_view.getSize().y + (simulation_view.getCenter().y - simulation_view.getSize().y / 2);

	//update mouse position relative to normal view
	mouseX_normal = ((float)sf::Mouse::getPosition(window).x / window.getSize().x) * normal_view.getSize().x - (normal_view.getSize().x - SCREEN_WIDTH) / 2;
	mouseY_normal = ((float)sf::Mouse::getPosition(window).y / window.getSize().y) * normal_view.getSize().y - (normal_view.getSize().y - SCREEN_HEIGHT) / 2;

	//move screen
	if (is_middle_mousebutton_down) {
		zoom_offset_x += mouseX - middle_mouse_pressed_pos_x;
		zoom_offset_y += mouseY - middle_mouse_pressed_pos_y;
		zoom_offset_x = zoom_offset_x < -SCREEN_WIDTH / 2 ? -SCREEN_WIDTH / 2 : zoom_offset_x > SCREEN_WIDTH / 2 ? SCREEN_WIDTH / 2 : zoom_offset_x;
		zoom_offset_y = zoom_offset_y < -SCREEN_HEIGHT / 2 ? -SCREEN_HEIGHT / 2 : zoom_offset_y > SCREEN_HEIGHT / 2 ? SCREEN_HEIGHT / 2 : zoom_offset_y;
		set_view_position(SCREEN_WIDTH / 2 - zoom_offset_x, SCREEN_HEIGHT / 2 - zoom_offset_y);
	}
}


//render to window
void Game::render()
{
	window.clear();
	window.setView(simulation_view);

	window.draw(background);



	//draw particles
	particle_circle.setRadius(lookupTable.particle_radius);
	particle_circle.setOrigin(lookupTable.particle_radius, lookupTable.particle_radius);
	if (!play_recording) {
		//line & circle
		sf::Vertex line[2];
		sf::CircleShape circle;
		circle.setFillColor(sf::Color::Transparent);
		circle.setOutlineColor(sf::Color::White);
		circle.setOutlineThickness(1);



		for (unsigned int index = 0; index < lookupTable.particle_count; index++) {
			particle_circle.setPosition(get_particle_position_X(index), get_particle_position_Y(index));
			particle_circle.setFillColor(lookupTable.get_Color(get_particle_ID(index)));
			window.draw(particle_circle);

			if (draw_hitbox) {
				if (index == 0) {
					circle.setRadius(lookupTable.max_influence_radius);
					circle.setOrigin(circle.getRadius(), circle.getRadius());
					circle.setPosition(particle_circle.getPosition());
					window.draw(circle);

					sf::RectangleShape r;
					float influence_rect_min_x = get_particle_position_X(0) - lookupTable.max_influence_radius;
					float influence_rect_max_x = get_particle_position_X(0) + lookupTable.max_influence_radius;
					float influence_rect_min_y = get_particle_position_Y(0) - lookupTable.max_influence_radius;
					float influence_rect_max_y = get_particle_position_Y(0) + lookupTable.max_influence_radius;
					r.setSize(sf::Vector2f(influence_rect_max_x - influence_rect_min_x, influence_rect_max_y - influence_rect_min_y));
					r.setPosition(influence_rect_min_x, influence_rect_min_y);
					r.setFillColor(sf::Color::Transparent);
					r.setOutlineColor(sf::Color::White);
					r.setOutlineThickness(3);
					window.draw(r);


				}

				line[0].position = sf::Vector2f(particle_circle.getPosition().x, particle_circle.getPosition().y);
				line[1].position = sf::Vector2f(particle_circle.getPosition().x + get_particle_velocity_X(index) * 6, particle_circle.getPosition().y + +get_particle_velocity_Y(index) * 6);
				window.draw(line, 2, sf::Lines);
			}

		}

		//circle around focused particle
		if (focused_particle_index > -1) {
			particle_circle.setPosition(get_particle_position_X(focused_particle_index), get_particle_position_Y(focused_particle_index));
			particle_circle.setFillColor(lookupTable.get_Color(get_particle_ID(focused_particle_index)));
			window.draw(particle_circle);
			circle.setRadius(lookupTable.particle_radius + 3);
			circle.setOrigin(circle.getRadius(), circle.getRadius());
			circle.setPosition(particle_circle.getPosition());
			window.draw(circle);
		}

		//grab radius
		if (!hide_hud) {
			circle.setRadius(mouse_grab_radius);
			circle.setOrigin(mouse_grab_radius, mouse_grab_radius);
			circle.setPosition(mouseX, mouseY);
			window.draw(circle);
		}


		//draw quadtree
		if (draw_quadtree) {
			qt_read->draw(window);
		}


	}
	else {//play recording
		
		//draw particles
		float x, y;
		unsigned int temp_pos_data = 0;
		for (int i = 0; i < loaded_particle_count; i++) {
			particle_circle.setFillColor(sf::Color(loaded_particle_color[i * 3], loaded_particle_color[i * 3 + 1], loaded_particle_color[i * 3 + 2], 255));
			temp_pos_data = 0;
			memcpy(&temp_pos_data, &loaded_position_data[current_frame * loaded_particle_count * 3 + i * 3], 3);

 			x = (float(temp_pos_data >> 12) / 4095) * SCREEN_WIDTH;
			y = (float(temp_pos_data & 0xFFF) / 4095) * SCREEN_HEIGHT;
			particle_circle.setPosition(x, y);

			window.draw(particle_circle);
		}


	}




	//normal view
	window.setView(normal_view);
	sf::Text text;
	text.setFont(font);
	text.setFillColor(sf::Color::White);


	//draw pause button
	if (simulation_paused && !hide_hud) {
		sf::RectangleShape rect;
		rect.setFillColor(sf::Color::White);
		rect.setOutlineColor(sf::Color::Transparent);
		rect.setSize(sf::Vector2f(20, 70));
		rect.setPosition(normal_view.getCenter().x + normal_view.getSize().x / 2 - 120, 50);
		window.draw(rect);
		rect.setPosition(normal_view.getCenter().x + normal_view.getSize().x / 2 - 80, 50);
		window.draw(rect);
	}


	//draw recording stuff
	if (recording) {
		rec_circle.setPosition(normal_view.getCenter().x + normal_view.getSize().x / 2 - 670, 90);
		window.draw(rec_circle);
		text.setString("Press 'Enter' to stop recording");
		text.setPosition(normal_view.getCenter().x + normal_view.getSize().x / 2 - 600, 70);
		text.setCharacterSize(30);
		window.draw(text);
	}

	//draw user input window
	if (show_user_input_window) {
		window.draw(user_input_plane);

		if (user_input_ID == 1) {//change particle count
			text.setString("Enter Particle-count and press 'Enter':");
			text.setPosition(500, 400);
			text.setCharacterSize(60);
			window.draw(text);
		}else if (user_input_ID == 2) {//change rec framerate
			text.setString("Enter framerate:");
			text.setPosition(750, 450);
			text.setCharacterSize(70);
			window.draw(text);
		}
		text.setString(std::to_string(user_input_value));
		text.setCharacterSize(80);
		text.setPosition(SCREEN_WIDTH/2 - std::to_string(user_input_value).length() * 20, 600);
		window.draw(text);
	}
	//draw menu
	else {
		if (show_menu) {
			text.setPosition(200, 200);

			//plane
			window.draw(menu_plane);

			//checkboxes
			window.draw(menu_screen_wrap_checkbox.rect_shape);
			text.setString("Border");
			text.setCharacterSize(40);
			text.setPosition(menu_screen_wrap_checkbox.pos_x + 100, menu_screen_wrap_checkbox.pos_y);
			window.draw(text);

			//buttons
			window.draw(menu_random_color_button.rect_shape);
			text.setString("C");
			text.setCharacterSize(40);
			text.setPosition(menu_random_color_button.pos_x + 10, menu_random_color_button.pos_y);
			window.draw(text);

			window.draw(menu_random_position_button.rect_shape);
			text.setString("P");
			text.setCharacterSize(40);
			text.setPosition(menu_random_position_button.pos_x + 10, menu_random_position_button.pos_y);
			window.draw(text);

			window.draw(menu_random_behavior_button.rect_shape);
			text.setString("B");
			text.setCharacterSize(40);
			text.setPosition(menu_random_behavior_button.pos_x + 10, menu_random_behavior_button.pos_y);
			window.draw(text);

			window.draw(menu_set_particle_count_button.rect_shape);
			text.setString("N");
			text.setCharacterSize(40);
			text.setPosition(menu_set_particle_count_button.pos_x + 10, menu_set_particle_count_button.pos_y);
			window.draw(text);

			//draw info box if mouse over button
			if (menu_random_color_button.mouse_over) {
				mouse_info_plane.setSize(sf::Vector2f(170, 30));
				mouse_info_plane.setPosition(mouseX_normal, mouseY_normal - 35);
				text.setString("Randomise Color");
				text.setCharacterSize(20);
				text.setPosition(sf::Vector2f(mouse_info_plane.getPosition().x + 3, mouse_info_plane.getPosition().y + 1));
				window.draw(mouse_info_plane);
				window.draw(text);
			}
			else if (menu_random_position_button.mouse_over) {
				mouse_info_plane.setSize(sf::Vector2f(190, 30));
				mouse_info_plane.setPosition(mouseX_normal, mouseY_normal - 35);
				text.setString("Randomise Position");
				text.setCharacterSize(20);
				text.setPosition(sf::Vector2f(mouse_info_plane.getPosition().x + 3, mouse_info_plane.getPosition().y + 1));
				window.draw(mouse_info_plane);
				window.draw(text);
			}
			else if (menu_random_behavior_button.mouse_over) {
				mouse_info_plane.setSize(sf::Vector2f(200, 30));
				mouse_info_plane.setPosition(mouseX_normal, mouseY_normal - 35);
				text.setString("Randomise Behavior");
				text.setCharacterSize(20);
				text.setPosition(sf::Vector2f(mouse_info_plane.getPosition().x + 3, mouse_info_plane.getPosition().y + 1));
				window.draw(mouse_info_plane);
				window.draw(text);
			}
			else if (menu_set_particle_count_button.mouse_over) {
				mouse_info_plane.setSize(sf::Vector2f(170, 30));
				mouse_info_plane.setPosition(mouseX_normal, mouseY_normal - 35);
				text.setString("Set particle count");
				text.setCharacterSize(20);
				text.setPosition(sf::Vector2f(mouse_info_plane.getPosition().x + 3, mouse_info_plane.getPosition().y + 1));
				window.draw(mouse_info_plane);
				window.draw(text);
			}

			//sliders
			window.draw(menu_max_velocity_slider.rect_shape);
			window.draw(menu_max_velocity_slider.circle_shape);
			text.setString("Velocity:");
			text.setCharacterSize(30);
			text.setPosition(menu_max_velocity_slider.pos_x + menu_max_velocity_slider.width + 40, menu_max_velocity_slider.pos_y - menu_max_velocity_slider.circle_shape.getRadius());
			window.draw(text);
			text.setString(std::to_string(lookupTable.max_velocity));
			text.setCharacterSize(30);
			text.setPosition(menu_max_velocity_slider.pos_x + menu_max_velocity_slider.width + 40, menu_max_velocity_slider.pos_y - menu_max_velocity_slider.circle_shape.getRadius() + 40);
			window.draw(text);

			window.draw(menu_friction_slider.rect_shape);
			window.draw(menu_friction_slider.circle_shape);
			text.setString("Friction:");
			text.setCharacterSize(30);
			text.setPosition(menu_friction_slider.pos_x + menu_friction_slider.width + 40, menu_friction_slider.pos_y - menu_friction_slider.circle_shape.getRadius());
			window.draw(text);
			text.setString(std::to_string(lookupTable.friction));
			text.setCharacterSize(30);
			text.setPosition(menu_friction_slider.pos_x + menu_friction_slider.width + 40, menu_friction_slider.pos_y - menu_friction_slider.circle_shape.getRadius() + 40);
			window.draw(text);

			window.draw(menu_max_force_slider.rect_shape);
			window.draw(menu_max_force_slider.circle_shape);
			text.setString("Force:");
			text.setCharacterSize(30);
			text.setPosition(menu_max_force_slider.pos_x + menu_max_force_slider.width + 40, menu_max_force_slider.pos_y - menu_max_force_slider.circle_shape.getRadius());
			window.draw(text);
			text.setString(std::to_string(lookupTable.max_force));
			text.setCharacterSize(30);
			text.setPosition(menu_max_force_slider.pos_x + menu_max_force_slider.width + 40, menu_max_force_slider.pos_y - menu_max_force_slider.circle_shape.getRadius() + 40);
			window.draw(text);

			window.draw(menu_max_influence_radius_slider.rect_shape);
			window.draw(menu_max_influence_radius_slider.circle_shape);
			text.setString("Influence-radius:");
			text.setCharacterSize(30);
			text.setPosition(menu_max_influence_radius_slider.pos_x + menu_max_influence_radius_slider.width + 40, menu_max_influence_radius_slider.pos_y - menu_max_influence_radius_slider.circle_shape.getRadius());
			window.draw(text);
			text.setString(std::to_string(int(lookupTable.max_influence_radius)));
			text.setCharacterSize(30);
			text.setPosition(menu_max_influence_radius_slider.pos_x + menu_max_influence_radius_slider.width + 40, menu_max_influence_radius_slider.pos_y - menu_max_influence_radius_slider.circle_shape.getRadius() + 40);
			window.draw(text);

			window.draw(menu_particle_radius_slider.rect_shape);
			window.draw(menu_particle_radius_slider.circle_shape);
			text.setString("Radius:");
			text.setCharacterSize(30);
			text.setPosition(menu_particle_radius_slider.pos_x + menu_particle_radius_slider.width + 40, menu_particle_radius_slider.pos_y - menu_particle_radius_slider.circle_shape.getRadius());
			window.draw(text);
			text.setString(std::to_string(lookupTable.particle_radius));
			text.setCharacterSize(30);
			text.setPosition(menu_particle_radius_slider.pos_x + menu_particle_radius_slider.width + 40, menu_particle_radius_slider.pos_y - menu_particle_radius_slider.circle_shape.getRadius() + 40);
			window.draw(text);

			window.draw(menu_particle_types_slider.rect_shape);
			window.draw(menu_particle_types_slider.circle_shape);
			text.setString("Types:");
			text.setCharacterSize(30);
			text.setPosition(menu_particle_types_slider.pos_x + menu_particle_types_slider.width + 40, menu_particle_types_slider.pos_y - menu_particle_types_slider.circle_shape.getRadius());
			window.draw(text);
			text.setString(std::to_string(new_particle_types_count));
			text.setCharacterSize(30);
			text.setPosition(menu_particle_types_slider.pos_x + menu_particle_types_slider.width + 140, menu_particle_types_slider.pos_y - menu_particle_types_slider.circle_shape.getRadius());
			window.draw(text);

			window.draw(menu_simulation_fps_slider.rect_shape);
			window.draw(menu_simulation_fps_slider.circle_shape);
			text.setString("FPS:");
			text.setCharacterSize(30);
			text.setPosition(menu_simulation_fps_slider.pos_x + menu_simulation_fps_slider.width + 40, menu_simulation_fps_slider.pos_y - menu_simulation_fps_slider.circle_shape.getRadius());
			window.draw(text);
			text.setString(std::to_string(int(simulation_fps)));
			text.setCharacterSize(30);
			text.setPosition(menu_simulation_fps_slider.pos_x + menu_simulation_fps_slider.width + 140, menu_simulation_fps_slider.pos_y - menu_simulation_fps_slider.circle_shape.getRadius());
			window.draw(text);
		}
		if (show_save_and_load_menu_window) {
			window.draw(save_and_load_plane);

			text.setString("Settings:");
			text.setCharacterSize(50);
			text.setPosition(save_and_load_plane.getPosition().x + 30, save_and_load_plane.getPosition().y + 20);
			window.draw(text);

			window.draw(save_settings_button.rect_shape);
			text.setString("save settings");
			text.setCharacterSize(30);
			text.setPosition(save_settings_button.pos_x + 10, save_settings_button.pos_y + 10);
			window.draw(text);

			window.draw(load_settings_button.rect_shape);
			text.setString("load settings");
			text.setCharacterSize(30);
			text.setPosition(load_settings_button.pos_x + 10, load_settings_button.pos_y + 10);
			window.draw(text);

			text.setString("Recording:");
			text.setCharacterSize(50);
			text.setPosition(save_and_load_plane.getPosition().x + 30, save_and_load_plane.getPosition().y + 300);
			window.draw(text);

			window.draw(save_recording_button.rect_shape);
			text.setString("record simulation");
			text.setCharacterSize(30);
			text.setPosition(save_recording_button.pos_x + 10, save_recording_button.pos_y + 10);
			window.draw(text);

			window.draw(load_recording_button.rect_shape);
			text.setString("load recording");
			text.setCharacterSize(30);
			text.setPosition(load_recording_button.pos_x + 10, load_recording_button.pos_y + 10);
			window.draw(text);
		}
	}

	//play recording screen
	if (play_recording && !hide_hud) {
		//draw button & slider
		window.draw(close_recording_button.rect_shape);
		text.setString("X");
		text.setCharacterSize(40);
		text.setPosition(close_recording_button.rect_shape.getPosition().x + 10, close_recording_button.rect_shape.getPosition().y);
		window.draw(text);


		window.draw(recording_framerate_button.rect_shape);
		text.setString("FPS");
		text.setCharacterSize(20);
		text.setPosition(recording_framerate_button.rect_shape.getPosition().x + 8, recording_framerate_button.rect_shape.getPosition().y);
		window.draw(text);
		text.setString(std::to_string(int(recording_fps)));
		text.setPosition(recording_framerate_button.rect_shape.getPosition().x + 8, recording_framerate_button.rect_shape.getPosition().y + 20);
		window.draw(text);



		//frame count
		//text.setString(std::to_string(current_frame));
		//text.setCharacterSize(50);
		//text.setPosition(200, 100);
		//window.draw(text);

		window.draw(recording_frame_slider.rect_shape);
		window.draw(recording_frame_slider.circle_shape);
	}


	//display drawn window to screen
	window.display();
}

//=======================================



// particle data Setter

void Game::set_particle_data(float* particle_buffer, int index, int id, float pos_x, float pos_y, float vel_x, float vel_y) {
	particle_buffer[index * number_of_particle_data_in_4bytes] = float(id);
	particle_buffer[index * number_of_particle_data_in_4bytes + 1] = pos_x;
	particle_buffer[index * number_of_particle_data_in_4bytes + 2] = pos_y;
	particle_buffer[index * number_of_particle_data_in_4bytes + 3] = vel_x;
	particle_buffer[index * number_of_particle_data_in_4bytes + 4] = vel_y;
}

void Game::set_particle_ID(float* particle_buffer, int index, int id) {
	particle_buffer[index * number_of_particle_data_in_4bytes] = float(id);
}

void Game::set_particle_position(float* particle_buffer, int index, float pos_x, float pos_y) {
	particle_buffer[index * number_of_particle_data_in_4bytes + 1] = pos_x;
	particle_buffer[index * number_of_particle_data_in_4bytes + 2] = pos_y;
}

void Game::set_particle_position_X(float* particle_buffer, int index, float pos_x) {
	particle_buffer[index * number_of_particle_data_in_4bytes + 1] = pos_x;
}

void Game::set_particle_position_Y(float* particle_buffer, int index,float pos_y) {
	particle_buffer[index * number_of_particle_data_in_4bytes + 2] = pos_y;
}

void Game::set_particle_velocity(float* particle_buffer, int index, float vel_x, float vel_y) {
	particle_buffer[index * number_of_particle_data_in_4bytes + 3] = vel_x;
	particle_buffer[index * number_of_particle_data_in_4bytes + 4] = vel_y;
}

void Game::set_particle_velocity_X(float* particle_buffer, int index, float vel) {
	particle_buffer[index * number_of_particle_data_in_4bytes + 3] = vel;
}

void Game::set_particle_velocity_Y(float* particle_buffer, int index, float vel) {
	particle_buffer[index * number_of_particle_data_in_4bytes + 4] = vel;
}

// particle data Getter

int Game::get_particle_ID(int index) {
	return particle_buffer[index * number_of_particle_data_in_4bytes];
}

float Game::get_particle_position_X(int index) {
	return particle_buffer[index * number_of_particle_data_in_4bytes + 1];
}

float Game::get_particle_position_Y(int index) {
	return particle_buffer[index * number_of_particle_data_in_4bytes + 2];
}

float Game::get_particle_velocity_X(int index) {
	return particle_buffer[index * number_of_particle_data_in_4bytes + 3];
}

float Game::get_particle_velocity_Y(int index) {
	return particle_buffer[index * number_of_particle_data_in_4bytes + 4];
}
