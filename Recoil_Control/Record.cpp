#include "Record.h"
#include "Utils.h"

/* read the recorded recoil graphs of a weapon, from file */
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

/* reads all recoil graphs stored in the graph folder */
std::vector<Recoil_Weapon> read_weapons() {
	std::vector<Recoil_Weapon> weapons;
	for (const auto& entry : std::filesystem::directory_iterator(record_path)) {

		std::string path = entry.path().string();
		std::vector<Record> record = read_record(path);

		std::size_t pos = path.find("\\") + 1;

		Recoil_Weapon rw;
		rw.name = path.substr(pos);
		rw.record = record;
		weapons.push_back(rw);
	}

	return weapons;
}

/* records a recoil pattern for the specified weapon */
void record() {

	// create the directory if it doesn't exist yet
	std::filesystem::create_directory("weapons");

	std::cout << "Weapon name - must correspond to the image file name of the state recognition (without the file ending):" << std::endl;

	std::string weapon_name;
	std::cin >> weapon_name;

	std::cout << "!!! WARNING: WILL HOOK INTO CSGO !!!" << std::endl << "hit TAB to start recording, ESC to cancel, and ENTER to finish. Simply fire an entire magazine with your weapon (without moving the mouse) and hit tab afterwards to safe the recording" << std::endl;

	while (true) {
		if (GetAsyncKeyState(VK_TAB) & 0x8000) {
			std::cout << "recording started!" << std::endl;
			break;
		}
		if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) {
			return;
		}
		Sleep(1);
	}

	DWORD pId = get_csgo_pid();
	HANDLE proc = OpenProcess(PROCESS_VM_READ, FALSE, pId);

	DWORD local_player = get_local_player(pId, proc);

	DWORD aimpunch_angles = local_player + netvars::m_aimPunchAngle;

	std::vector<Record> records;

	float previous_punch_x = 0;
	float previous_punch_y = 0;

	std::chrono::time_point<std::chrono::high_resolution_clock> start_ts = std::chrono::high_resolution_clock::now();

	bool start = false;
	while (true) {
		if (GetAsyncKeyState(VK_LBUTTON) & 0x8000) {

			Vector_2d punch;
			ReadProcessMemory(proc, (DWORD*)aimpunch_angles, &punch, sizeof(Vector_2d), NULL);

			Record r;
			r.timestamp = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - start_ts).count();
			r.aimpunch_x = punch.x;
			r.aimpunch_y = punch.y;

			records.push_back(r);

			previous_punch_x = punch.x;
			previous_punch_y = punch.y;

			// afterwards we will need to process these recordings, we can't record too fast otherwise 
			// we can't sync up properly in our recoil compensation
			// increase until recoil doesnt call wtf in std::cout
			nanosleep(1600000);
		}else if (GetAsyncKeyState(VK_RETURN) & 0x8000) {
			std::cout << "recording finished!" << std::endl;
			break;
		}
	}

	std::ofstream file((record_path + weapon_name).c_str(), std::ios::out | std::ios::binary);

	int size = sizeof(Record) * records.size();
	file.write((char*)records.data(), size);
	file.close();

	std::cout << "recording saved!" << std::endl;
}