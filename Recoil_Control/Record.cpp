#include "Record.h"
#include "Utils.h"

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