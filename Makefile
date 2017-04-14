PLAT=none
PLATS=mingw linux chip8 emscripten

export CPP_SRC=src/*.cpp
export CPP_H=src/*.h
export C_SRC=src/emu2413/*.c
export C_H=src/emu2413/*.c
export C_OBJ=emu2413.o

all: $(PLAT)

$(PLATS) clean:
	$(MAKE) -f Makefile.$@
	
none:
	@echo Build with 'make PLATFORM'. The supported platforms are:
	@echo   $(PLATS)
