#include <iostream>
#include <thread>
#include <string>
#include <ctime>
#include <sstream>
#include <condition_variable>
#include <mutex>
#include <chrono>
#include "Game.h"
#include "IO_DATA.h"



void print_help() {
	using namespace std;
	cout << "\n       ===== Keys: =====\n";
	cout << "   Space: pause/play\n";
	cout << "   F11: Fullscreen\n";
	cout << "   Left Alt: Menu\n";
	cout << "   P: randomise position\n";
	cout << "   B: randomise behavior\n";
	cout << "   C: randomise color\n";
	cout << "   W: toggle Border\n";
	cout << "   N: change number of particles\n";
	cout << "   Tab: place particles in mouse-circle\n";
	cout << "   L / Ctrl+S: Save & Load menu\n";
	cout << "   H: show debug-data (velocity/influence radius)\n";
	cout << "   D: Do extra pause between frames (to not overheat cpu when simulating many particles)\n";

	cout << "\n       ===== Mouse: =====\n";
	cout << "   middle mousebutton: drag screen\n";
	cout << "   mousewheel: resize mouse-circle\n";
	cout << "   mousewheel + Ctrl: zoom\n";
	cout << "   right mousebutton: focus on particle / reset view\n";
	cout << "   left mousebutton: drag particles" << endl;
}


void run_particle_computation_thread(Game* game, int thread_index) {
	game->th_compute_particles(thread_index);
}

void run_update_particle_manager_thread(Game* game) {
	game->th_manage_particle_update();
}


int main(int argc, char* argv[]) {


	//=========================================
	//============      GAME      =============
	//=========================================

	//probably only works with VS C++ 14 compiler

	int thread_count = 1;
	bool bad_number = true;
	while (bad_number) {
		std::cout << std::endl;
		std::cout << " How many threads? (1-10):";
		std::string input;
		std::cin >> input;
		try {
			thread_count = std::stoi(input);
			bad_number = false;
		}
		catch (const std::exception& e) {
			system("cls");
			std::cout << " Enter a number between 1 and 10!" << std::endl;

		}
	}
	int max_threads_anzahl = 10;
	thread_count = thread_count < 1 ? 1 : thread_count > max_threads_anzahl ? max_threads_anzahl : thread_count;


	system("cls");
	std::cout << " using " << thread_count << " thread(s)" << std::endl;
	std::cout << std::endl;

	print_help();

	Game game(thread_count, 3723);//3723

	//load is opened with file
	if (argc == 2) {
		//TODO: load settingsfile at startup
		std::cout << "opening file..." << std::endl;
		std::cout << argv[1] << std::endl;
		game.load_file_from_start(argv[1]);
	}

	//start particle manager thread
	std::thread update_particle_manager_thread(run_update_particle_manager_thread, &game);
	update_particle_manager_thread.detach();

	//start all computation-threads
	std::vector<std::thread> thread_list;
	for (int i = 0; i < thread_count; i++) {
		thread_list.push_back( std::thread(run_particle_computation_thread, &game, i));
		thread_list.at(i).detach();
	}



	game.run();

	return 0;
}