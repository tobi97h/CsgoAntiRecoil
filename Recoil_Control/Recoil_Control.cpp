/* needs to come first

winsock2 redefinition different linkage

I believe you are getting these errors because windows.h will include winsock.h.
Reverse the order of your includes so that WinSock2.h comes before windows.h. 
WinSock2.h has some #defines in it to keep windows.h from including winsock.h

*/

#include "Listener.h"

#include "Utils.h"
#include "Record.h"
#include "Recoil.h"
#include "Calibrate.h"

int main()
{

	while (true) {

		std::cout << "1. Calibrate - Determines vector to mouse movement factor" << std::endl
			<< "2. Record recoil graph - Empty a magazine of your weapon of choice" << std::endl
			<< "3. RCS - Start State_Recognition once promptet" << std::endl;

		int option;

		bool query = true;
		while (query) {
			for (int i = 0; i < 9; i++) {

				if (GetAsyncKeyState(0x30 + i) & 0x8000) {
					std::cout << "chose option: " << i << std::endl;
					query = false;
					option = i;
					break;
				}
			}
			// tickrate query
			Sleep(1);
		}
		// some time to release the key
		Sleep(500);

		switch (option)
		{
		case 1:
			calibrate();
			break;
		case 2:
			record();
			break;
		case 3:
			Recoil r;
			Listener l(&r);
			l.thread.join();
			r.thread.join();
			break;
		}
	}
    
}
