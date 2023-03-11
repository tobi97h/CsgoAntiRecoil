# Project

This project provides anti recoil for the game csgo. It uses custom generated vectors and open cv for detecting the currently selected weapon. This is to prevent hooking into the game triggering anti cheat services.

# Prep CSGO

* Set Project -> Properties -> General -> Advanced Set option to "Use Unicode Character Set"(Multi-Byte Character Set)

* Turn off mouse coalesce in windows settings

* (Turn of csgo mouse wheel weapon swapping - otherwise it might get arkward)

* `+fps_max 144` to launch options

* `cl_showloadout 1` incase hud randomly dissapears

* flashing weapons for example in warmups are a problem for detection. Simply go for headshot training here

# Concept

## Python Opencv

* Has Screenshots of all the weapon toolbars

* Regualarly takes screencap of csgo and feature matches against the toolbars

* Sends best match to c++ recoil algo

## C++ Recoil Algo

* Ability to record recoil vectors

* Ability to emulate mouse movements, nullifying the recoil vectors - without hooking in the game

# Important!

* recordings need to have the same name like screenshots of the weapons for matching (without the .PNG file ending)