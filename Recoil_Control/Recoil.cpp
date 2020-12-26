#include "Recoil.h"

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

		// unpause when swapping to main weapon - NUM 1
		if (GetAsyncKeyState(0x31) & 0x8000) {
			paused = false;
			std::cout << "paused: " << paused << std::endl;
		}

		// pause when swapping to others - NUM 2 - 6
		for (int i = 0; i < 5; i++) {
			if (GetAsyncKeyState(0x32 + i) & 0x8000) {
				paused = true;
				std::cout << "paused: " << paused << std::endl;
				break;
			}
		}

		// buying (B) || manually (P) || Swap Q
		if (GetAsyncKeyState(0x42) & 0x8000 || GetAsyncKeyState(0x50) & 0x8000 || GetAsyncKeyState(0x51) & 0x8000) {
			paused = !paused;
			std::cout << "paused: " << paused << std::endl;
			Sleep(500);
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

			// only move if we can move more than one px
			if (fullx_px > 1 || fullx_px < -1) {

				move(fullx_px, 0);

				// remove the full pixels from the pool since they have been moved
				movex_px_pool -= fullx_px;
			}

			if (fully_px > 1 || fully_px < -1) {

				move(0, fully_px);

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
			std::cout << i + 1 << ". " << rw.path << " num. records: " << rw.record.size() << std::endl;
		}

		int weapon;
		std::cout << std::endl << "select weapon: " << std::endl;

		bool query = true;
		while (query) {
			for (int i = 0; i < 9; i++) {
				if (GetAsyncKeyState(0x30 + i) & 0x8000) {
					std::cout << i << ": " << weapons[i - 1].path << std::endl;
					query = false;
					weapon = i - 1;
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
