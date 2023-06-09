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

			float move_x_px_pool = 0;
			float move_y_px_pool = 0;

			// iterate here as fast as possible
			for (int i = 1; i < weapon->record.size() && GetAsyncKeyState(VK_LBUTTON) & 0x8000 && current_weapon.load() != NULL; i++) {

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
				move_x_px_pool += change_x * factor_x * 2;
				move_y_px_pool += change_y * factor_y * 2;

				// x adjust
				float full_x_px = floor(move_x_px_pool);

				// y adjust
				float full_y_px = floor(move_y_px_pool);

				bool move_x = full_x_px > 1 || full_x_px < -1;
				bool move_y = full_y_px > 1 || full_y_px < -1;

				if (move_x && move_y) {
					move(full_x_px, full_y_px);
					// remove the full pixels from the pool since they have been moved
					move_x_px_pool -= full_x_px;
					move_y_px_pool -= full_y_px;
				}
				else if (move_x) {
					move(full_x_px, 0);
					move_x_px_pool -= full_x_px;
				}
				else if (move_y) {
					move(0, full_y_px);
					move_y_px_pool -= full_y_px;
				}

				LONGLONG passed = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - start_ts).count();
				LONGLONG left = ns_diff_recordings - passed;
				if (left > 0) {
					nanosleep(left);
				}
				else {
					std::cout << "WTF Recording distance to low - can't keep syned! record weapon with higher nano sleep!!" << std::endl;
				}
			}
		}
	}
}
void Recoil::set_current(const char* id) {
	Recoil_Weapon* current = current_weapon.load();

	if (strcmp("nothing matched", id) == 0) {
		if (current != NULL) {
			current_weapon.store(NULL);
			std::cout << "nothing matched" << std::endl;
		}
		return;
	}

	bool found = false;
	for (int i = 0; i < weapons.size(); i++) {
		Recoil_Weapon* rw = &weapons[i];
		if (strcmp(rw->name.c_str(), id) == 0) {
			// found the weapon
			if (rw != current) {
				current_weapon.store(rw);
				std::cout << "set new current:" << rw->name << std::endl;
			}
			found = true;
			break;
		}
	}
	if (!found) {
		std::cerr << "no fitting recording found: " << id << std::endl;
	}
};

Recoil::Recoil() {
	if (!std::filesystem::exists(cal_file)) {
		std::cerr << "calibrate file missing" << std::endl;
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
		std::cerr << "calibrate file corrupt !" << std::endl;
	}
	thread = std::thread(&Recoil::loop, this);
}

