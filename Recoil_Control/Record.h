#pragma once

#include <fstream>
#include <vector>
#include <string>
#include "Windows.h"
#include <filesystem>
#include <iostream>

struct Record {
	LONGLONG timestamp;
	float aimpunch_x;
	float aimpunch_y;
};

struct Recoil_Weapon {
	std::vector<Record> record;
	std::string name;
};

std::vector<Record> read_record(std::string filepath);

std::vector<Recoil_Weapon> read_weapons();

void record();