# Prep CSGO

* Set Project -> Properties -> General -> Advanced Set option to "Use Unicode Character Set"(Multi-Byte Character Set)

* Turn off mouse coalesce in windows settings

* (Turn of csgo mouse wheel weapon swapping - otherwise it might get arkward)

* "fps_max 144" in auto cfg of csgo 


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