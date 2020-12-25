

#include <iostream>
#include "Windows.h"
#include "winuser.h"
#include "TlHelp32.h"
#include "Offsets.cpp"
#include <math.h>   

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

struct Vector_2d { float x, y; }; // vector structure



int main()
{

    /*
    double display_height = 1080;

    double display_width = 1920;

    double vertical_fov_degrees = 75;

    float factor = display_height / vertical_fov_degrees;
    */

    HWND hWnd = FindWindow(0, L"Counter-Strike: Global Offensive");
    DWORD pId;
    GetWindowThreadProcessId(hWnd, &pId);
    HANDLE proc = OpenProcess(PROCESS_VM_READ, FALSE, pId);
    
    DWORD client_dll = GetModuleBaseAddress(pId, L"client.dll");
    DWORD engine_dll = GetModuleBaseAddress(pId, L"engine.dll");

    DWORD local_player_offset = client_dll + signatures::dwLocalPlayer;

    DWORD local_player;

    ReadProcessMemory(proc, (DWORD*)local_player_offset, &local_player, sizeof(DWORD), 0);

    DWORD aimpunch_angles = local_player + netvars::m_aimPunchAngle;

    DWORD state_offset = engine_dll + signatures::dwClientState;

    DWORD state;

    ReadProcessMemory(proc, (DWORD*)state_offset, &state, sizeof(DWORD), NULL);

    DWORD view_angles = state + signatures::dwClientState_ViewAngles;
 
;
    float move_px_pool = 0;
    float previous_punch = 0;

    while (true) {
        Vector_2d punch;
        ReadProcessMemory(proc, (DWORD*)aimpunch_angles, &punch, sizeof(Vector_2d), NULL);

        // capture the change of the punch vector
        float change = previous_punch - punch.x;
        previous_punch = punch.x;

        // add the to the amount of pixels (with decimal points) that should be moved
        move_px_pool += change * 108;

        std::cout << "punch: " << punch.x << std::endl;
        std::cout << "change: " << change << std::endl;
        std::cout << "move_px_pool: " << move_px_pool << std::endl;
      
        LONG full_pixels = floor(move_px_pool);
        
        if (full_pixels > 1 || full_pixels < -1) {
            std::cout << "moving full_pixels: " << full_pixels << std::endl;
            INPUT tp;
            tp.type = INPUT_MOUSE;
            tp.mi.dwFlags = MOUSEEVENTF_MOVE;
            tp.mi.mouseData = NULL;
            tp.mi.dwExtraInfo = NULL;
            tp.mi.dx = 0;
            tp.mi.dy = full_pixels;
            tp.mi.time = NULL;

            SendInput(1, &tp, sizeof(tp));

            // remove the full pixels from the pool since they have been moved
            move_px_pool -= full_pixels;
        }

        
        Sleep(1);
    }
   
}
