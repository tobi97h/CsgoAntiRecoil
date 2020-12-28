#include "Recoil.h"


void Recoil::loop() {
	while (true) {
		// default windows sleep is fine
		Sleep(1);
		if (current_weapon.load() == NULL) {
			continue;
		}
		SHORT state = GetAsyncKeyState(VK_LBUTTON);

		if (state & 0x8000) {

			Recoil_Weapon* weapon = current_weapon.load();

			float movex_px_pool = 0;
			float movey_px_pool = 0;

			// iterate here as fast as possible
			for (int i = 1; i < weapon->record.size() && GetAsyncKeyState(VK_LBUTTON) & 0x8000; i++) {

				auto start_ts = std::chrono::high_resolution_clock::now();

				// when mouse is continously pressed
				Record previous = weapon->record[i - 1];
				Record current = weapon->record[i];

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
}
void Recoil::set_current(const char* id) {
	if (strcmp("nothing matched", id) == 0) {
		current_weapon.store(NULL);
		return;
	}

	bool found = false;
	for (int i = 0; i < weapons.size(); i++) {
		Recoil_Weapon* rw = &weapons[i];
		if (strcmp(rw->name.c_str(), id) == 0) {
			// found the weapon
			current_weapon.store(rw);
			std::cout << "set current:" << rw->name << std::endl;
			found = true;
			break;
		}
	}
	if (!found) {
		std::cout << "no fitting recording found: " << id << std::endl;
	}
};

Recoil::Recoil() {
	if (!std::filesystem::exists(cal_file)) {
		std::cerr << "calibrate first" << std::endl;
		return;
	}
	// read calibration factors
	std::ifstream file(cal_file, std::ios::in | std::ios::binary);
	file.seekg(0, std::ios::end);
	int filesize = file.tellg();

	file.seekg(0, std::ios::beg);


	file.read((char*)&factor_x, sizeof(float));
	file.read((char*)&factor_y, sizeof(float));

	if (this->factor_x == 0 || this->factor_y == 0) {
		std::cerr << "not calibrated !" << std::endl;
	}
	thread = std::thread(&Recoil::loop, this);
}

