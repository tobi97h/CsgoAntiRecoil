#include <iostream>

#include "Utils.h"
#include "Record.h"
#include "Recoil.h"
#include "Calibrate.h"


int main()
{
	
	while (true) {

		std::cout << "1. Calibrate - Go ingame and just hold still" << std::endl
			<< "2. Record Weapon" << std::endl
			<< "3. RCS - Backspace to select other weapon" << std::endl;

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
			// dont spam loop
			Sleep(1);
		}
		// time to release the key
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
			recoil();
			break;
		}
	}
}
