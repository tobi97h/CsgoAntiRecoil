# Prep CSGO

* Set Project -> Properties -> General -> Advanced Set option to "Use Unicode Character Set"(Multi-Byte Character Set)

* Turn off mouse coalesce in windows settings

* (Turn of csgo mouse wheel weapon swapping - otherwise it might get arkward)

* "fps_max 144" in auto cfg of csgo 

# install opencv

* `git clone https://github.com/microsoft/vcpkg`

* run `bootstrap-vcpkg.bat` && `vcpkg integrate install`

* run `vcpkg install opencv[contrib]:x86-windows` 