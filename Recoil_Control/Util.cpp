#include <iostream>
#include "Windows.h"
#include "TlHelp32.h"
#include "Offsets.cpp"
#include <chrono>
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
