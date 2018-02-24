# Prototracker-OPLL

This is the OPLL (YM2413) emulation fork of [Prototracker](https://github.com/kometbomb/prototracker).

Uses emu2413.c by Mitsutaka Okazaki 2001

## Building

Simply do ```make mingw``` where "mingw" is your platform of choice. Do ```make``` to see the supported platforms.

### Building for OSX

To install SDL/SDL2 on OSX(using brew), type brew install sdl2 sdl sdl_image sdl_mixer sdl_ttf portmidi. If you are getting the fatal error: 'SDL_image.h' file not found error, type apt install libsdl2-image-dev. You should then be able to run make linux.

The binary needs the files in the assets/ directory to work properly.
