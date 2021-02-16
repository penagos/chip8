#!/bin/sh
g++ *.cpp  -lSDL2_image `sdl2-config --libs --cflags`
