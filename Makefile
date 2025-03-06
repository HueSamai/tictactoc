RAYLIB_INCLUDE_DIR= C:\raylib\raylib\src
RAYLIB_LIB_DIR =C:\raylib\raylib\src

all: build test

build:
	gcc main.c -o main.exe -I$(RAYLIB_INCLUDE_DIR) -L$(RAYLIB_LIB_DIR) -lraylib -lopengl32 -lgdi32 -lwinmm -std=c99 -Wall -mwindows

test:
	main.exe
