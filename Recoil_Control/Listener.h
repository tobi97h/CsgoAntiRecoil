#pragma once

#define WIN32_LEAN_AND_MEAN
#include <thread>

#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>

#pragma comment (lib, "Ws2_32.lib")

#include "Recoil.h"

// Listens for informations on the selected weapon from the State_Recognition Project and sets the 
// currently selected weapon in the recoil compensation loop
class Listener {
private:
    SOCKET listen_socket = INVALID_SOCKET;
    bool keep_running = true;
    void loop();
    Recoil* recoil = NULL;
public:
    std::thread thread;
   
    Listener(Recoil* recoil);
};