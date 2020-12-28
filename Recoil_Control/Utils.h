#pragma once

#include "Windows.h"
#include <thread>
#include <string>
#include "TlHelp32.h"
#include "Offsets.h"

uintptr_t GetModuleBaseAddress(DWORD procId, const wchar_t* modName);

DWORD get_csgo_pid();

DWORD get_local_player(DWORD pId, HANDLE proc);

DWORD get_client_state(DWORD pId, HANDLE proc);

// custom nanosleep function, since default tickrate is too high in windows
BOOLEAN nanosleep(LONGLONG ns);

struct Vector_2d { float y, x; };

void move(float x, float y);

const std::string record_path = "weapons\\";

const std::string cal_file = "calibration";