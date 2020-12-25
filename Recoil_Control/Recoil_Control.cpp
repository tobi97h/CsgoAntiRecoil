

#include <iostream>
#include "Windows.h"
#include "winuser.h"
#include "TlHelp32.h"
#include "Offsets.cpp"
#include <math.h>   
#include <chrono>
#include <vector>
#include <fstream>
#include <string>
#include <filesystem>
#include <map>
#include <thread>


uintptr_t GetModuleBaseAddress(DWORD procId, const wchar_t* modName)
{
	uintptr_t modBaseAddr = 0;
	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, procId);
	if (hSnap != INVALID_HANDLE_VALUE)
	{
		MODULEENTRY32 modEntry;
		modEntry.dwSize = sizeof(modEntry);
		if (Module32First(hSnap, &modEntry))
		{
			do
			{
				if (!_wcsicmp(modEntry.szModule, modName))
				{
					modBaseAddr = (uintptr_t)modEntry.modBaseAddr;
					break;
				}
			} while (Module32Next(hSnap, &modEntry));
		}
	}
	CloseHandle(hSnap);
	return modBaseAddr;
}

DWORD get_csgo_pid() {
	HWND hWnd = FindWindow(0, L"Counter-Strike: Global Offensive");
	DWORD pId;
	GetWindowThreadProcessId(hWnd, &pId);
	return pId;
}

DWORD get_local_player(DWORD pId, HANDLE proc) {
	DWORD client_dll = GetModuleBaseAddress(pId, L"client.dll");

	DWORD local_player_offset = client_dll + signatures::dwLocalPlayer;

	DWORD local_player;

	ReadProcessMemory(proc, (DWORD*)local_player_offset, &local_player, sizeof(DWORD), 0);

	return local_player;
}

DWORD get_client_state(DWORD pId, HANDLE proc) {
	DWORD engine_dll = GetModuleBaseAddress(pId, L"engine.dll");

	DWORD state_offset = engine_dll + signatures::dwClientState;

	DWORD state;

	ReadProcessMemory(proc, (DWORD*)state_offset, &state, sizeof(DWORD), NULL);


	return state;
}

BOOLEAN nanosleep(LONGLONG ns) {
	auto start_ts = std::chrono::high_resolution_clock::now();
	LONGLONG passed;
	do {
		std::this_thread::sleep_for(std::chrono::nanoseconds(0));
		passed = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - start_ts).count();
	} while (passed <= ns);
	return TRUE;
}

std::string cal_file = "calibration";
std::string record_path = "weapons\\";

struct Vector_2d { float y, x; };

struct Record {
	LONGLONG timestamp;
	float aimpunch_x;
	float aimpunch_y;
};

struct Recoil_Weapon {
	std::vector<Record> record;
	std::string path;
};

std::vector<Record> read_record(std::string filepath) {

	std::vector<Record> records;

	std::ifstream file(filepath, std::ios::in | std::ios::binary);
	file.seekg(0, std::ios::end);
	int filesize = file.tellg();

	file.seekg(0, std::ios::beg);

	Record r;

	while (file.read((char*)&r, sizeof(Record))) {
		records.push_back(r);
	}

	return records;

}

std::vector<Recoil_Weapon> read_weapons() {
	std::vector<Recoil_Weapon> weapons;
	for (const auto& entry : std::filesystem::directory_iterator(record_path)) {

		std::string path = entry.path().string();
		std::vector<Record> record = read_record(path);

		Recoil_Weapon rw;
		rw.path = path;
		rw.record = record;
		weapons.push_back(rw);
	}

	return weapons;
}

void move(float x, float y) {
	INPUT tp;
	tp.type = INPUT_MOUSE;
	tp.mi.dwFlags = MOUSEEVENTF_MOVE | MOUSEEVENTF_MOVE_NOCOALESCE;
	tp.mi.mouseData = NULL;
	tp.mi.dwExtraInfo = NULL;
	tp.mi.time = NULL;
	tp.mi.dx = x;
	tp.mi.dy = y;
	SendInput(1, &tp, sizeof(tp));
	
}


void apply_records(std::vector<Record> records) {

	if (!std::filesystem::exists(cal_file)) {
		std::cout << "calibrate first" << std::endl;
		return;
	}

	// read calibration factors
	std::ifstream file(cal_file, std::ios::in | std::ios::binary);
	file.seekg(0, std::ios::end);
	int filesize = file.tellg();

	file.seekg(0, std::ios::beg);

	float factor_x;
	float factor_y;

	file.read((char*)&factor_x, sizeof(float));
	file.read((char*)&factor_y, sizeof(float));

	bool paused = false;

	while (true) {
		// atleast somewhat drosseln
		nanosleep(1);

		// oberer seitlicher mouse button - swap weapon / pause
		if (GetAsyncKeyState(VK_XBUTTON2) & 0x8000) {
			break;
		}

		// unpause when swapping to main weapon
		if (GetAsyncKeyState(0x31) & 0x8000) {
			paused = false;
		}

		// pause when swapping to others
		for (int i = 0; i < 3; i++) {
			if (GetAsyncKeyState(0x32 + i) & 0x8000) {
				paused = true;
				break;
			}
		}

		float movex_px_pool = 0;
		float movey_px_pool = 0;

		// iterate here as fast as possible
		for (int i = 1; i < records.size() && GetAsyncKeyState(VK_LBUTTON) & 0x8000 && !paused; i++) {

			auto start_ts = std::chrono::high_resolution_clock::now();


			// when mouse is continously pressed
			Record previous = records[i - 1];
			Record current = records[i];

			LONGLONG ns_diff_recordings = current.timestamp - previous.timestamp;

			float change_x = previous.aimpunch_x - current.aimpunch_x;
			float change_y = previous.aimpunch_y - current.aimpunch_y;

			// since aim punch never goes up, so its one dimensional?
			// you somehow have to multiply it by two to get the changes
			// that need to be reflected in the view angles
			movex_px_pool += change_x * factor_x * 2;
			movey_px_pool += change_y * factor_y * 2;

			// x adjust
			float fullx_px = floor(movex_px_pool);

			// y adjust
			float fully_px = floor(movey_px_pool);

			if (fullx_px > 1 || fullx_px < -1) {

				move(fullx_px, 0);

				// remove the full pixels from the pool since they have been moved
				movex_px_pool -= fullx_px;
			}

			if (fully_px > 1 || fully_px < -1) {

				move(0, fully_px);

				// remove the full pixels from the pool since they have been moved
				movey_px_pool -= fully_px;
			}
			

			LONGLONG passed = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - start_ts).count();
			LONGLONG left = ns_diff_recordings - passed;
			if (left > 0) {
				nanosleep(left);
			}
		}
	}

}


void recoil() {

	std::vector<Recoil_Weapon> weapons = read_weapons();

	while (true) {
		for (int i = 0; i < weapons.size(); i++) {
			Recoil_Weapon rw = weapons[i];
			std::cout << i +1 << ". " << rw.path << " num. records: " << rw.record.size() << std::endl;
		}

		int weapon;
		std::cout << std::endl << "select weapon: " << std::endl;

		bool query = true;
		while (query) {
			for (int i = 0; i < 9; i++) {		
				if (GetAsyncKeyState(0x30 + i) & 0x8000) {
					std::cout << i  << ": " << weapons[i-1].path <<std::endl;
					query = false;
					weapon = i-1;
				}
			}
			Sleep(1);
		}
		
		// wait key up
		Sleep(500);


		if (weapon < 0 || weapon > weapons.size() - 1) {
			std::cout << "invalid index" << std::endl;
			continue;
		}

		Recoil_Weapon rw = weapons[weapon];

		apply_records(rw.record);

	}
}

void record() {

	std::filesystem::create_directory("weapons");

	std::cout << "enter weapon name:" << std::endl;
	std::string weapon_name;
	std::cin >> weapon_name;

	DWORD pId = get_csgo_pid();
	HANDLE proc = OpenProcess(PROCESS_VM_READ, FALSE, pId);

	DWORD local_player = get_local_player(pId, proc);

	DWORD aimpunch_angles = local_player + netvars::m_aimPunchAngle;

	std::vector<Record> records;

	float previous_punch_x = 0;
	float previous_punch_y = 0;

	std::cout << "now get ingame. hit TAB to start, and empty a full magazine without moving the mouse. When finished hit TAB" << std::endl;

	std::chrono::time_point<std::chrono::high_resolution_clock> start_ts;
	bool start = false;
	while (true) {
		if (GetAsyncKeyState(VK_LBUTTON) & 0x8000 && start) {

			Vector_2d punch;
			ReadProcessMemory(proc, (DWORD*)aimpunch_angles, &punch, sizeof(Vector_2d), NULL);

			Record r;
			r.timestamp = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - start_ts).count();
			r.aimpunch_x = punch.x;
			r.aimpunch_y = punch.y;

			records.push_back(r);

			previous_punch_x = punch.x;
			previous_punch_y = punch.y;

			// 2/10tel einer millisekunde - fühlt sich nach sweetspot für meinen pc an
			nanosleep(200000);
		}

		else if (GetAsyncKeyState(VK_TAB) & 0x8000) {
			if (start) {
				break;
			}
			else {
				start = true;
				// give some time for the player to release the key, otherwise this will trigger multiple times
				Sleep(500);
				std::cout << "will record once you start shooting" << std::endl;
				start_ts = std::chrono::high_resolution_clock::now();
			}
		}

	}

	std::ofstream file((record_path + weapon_name).c_str(), std::ios::out | std::ios::binary);

	int size = sizeof(Record) * records.size();
	file.write((char*)records.data(), size);

	file.close();
	std::cout << "weapon recorded!" << std::endl;
}



void calibrate() {

	DWORD pId = get_csgo_pid();
	HANDLE proc = OpenProcess(PROCESS_VM_READ, FALSE, pId);

	DWORD state = get_client_state(pId, proc);
	DWORD view_angles = state + signatures::dwClientState_ViewAngles;

	std::cout << "calibrating in 5" << std::endl;
	Sleep(1000);
	std::cout << "calibrating in 4" << std::endl;
	Sleep(1000);
	std::cout << "calibrating in 3" << std::endl;
	Sleep(1000);
	std::cout << "calibrating in 2" << std::endl;
	Sleep(1000);
	std::cout << "calibrating in 1" << std::endl;
	Sleep(1000);
	// give the player some time to switch to the csgo window
	
	std::cout << "calibrating" << std::endl;

	Vector_2d view1;
	ReadProcessMemory(proc, (DWORD*)view_angles, &view1, sizeof(Vector_2d), NULL);

	INPUT tp;
	tp.type = INPUT_MOUSE;
	tp.mi.dwFlags = MOUSEEVENTF_MOVE | MOUSEEVENTF_MOVE_NOCOALESCE;
	tp.mi.mouseData = NULL;
	tp.mi.dwExtraInfo = NULL;
	tp.mi.dx = 100l;
	tp.mi.dy = 100l;
	tp.mi.time = NULL;

	SendInput(1, &tp, sizeof(tp));

	// give mouse event time to process
	Sleep(1000);
	Vector_2d view2;
	ReadProcessMemory(proc, (DWORD*)view_angles, &view2, sizeof(Vector_2d), NULL);

	float diff_x = view2.x - view1.x;
	float diff_y = view2.y - view1.y;

	float x_factor = 100.0f / diff_x;
	float y_factor = 100.0f / diff_y;

	// save the results to file
	std::ofstream file(cal_file, std::ios::out | std::ios::binary);

	file.write((char*)&x_factor, sizeof(float));
	file.write((char*)&y_factor, sizeof(float));

	file.close();

	std::cout << "calibrated" << std::endl;
}



int main()
{
	while (true) {

		std::cout << "1. Calibrate - Go ingame and just hold still" << std::endl
			<< "2. Record Weapon" << std::endl
			<< "3. RCS - Backspace to select other weapon" << std::endl;

		int option;

		bool query = true;
		while (query) {
			for (int i = 0; i < 9; i++) {

				if (GetAsyncKeyState(0x30 + i) & 0x8000) {
					std::cout << "chose option: " << i << std::endl;
					query = false;
					option = i;
					break;
				}
			}
			// dont spam loop
			Sleep(1);
		}
		// time to release the key
		Sleep(500);

		switch (option)
		{
		case 1:
			calibrate();
			break;
		case 2:
			record();
			break;
		case 3:
			recoil();
			break;
		}
	}

}
