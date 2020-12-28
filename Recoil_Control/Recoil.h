#pragma once
#include <vector>
#include "Record.h"
#include "Utils.h"
#include <thread>

// Recoil compensation handeler
// starts seperate thread
class Recoil {
private:
	float factor_x = 0;
	float factor_y = 0;
	std::atomic<Recoil_Weapon*> current_weapon = std::atomic<Recoil_Weapon*>(NULL);
	std::vector<Recoil_Weapon> weapons = read_weapons();
	void loop();
	
public:
	std::thread thread;
	Recoil();
	void set_current(const char* id);
};
