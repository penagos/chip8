#!/bin/sh
g++ *.cpp  -g -O0  -lSDL2_image `sdl2-config --libs --cflags`
