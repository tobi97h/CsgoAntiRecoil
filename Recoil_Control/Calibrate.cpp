#include "Calibrate.h"
#include "Utils.h"

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


