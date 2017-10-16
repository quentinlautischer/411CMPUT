
OS := $(shell uname -s)

ifeq ($(OS), Linux)
  FLAGS = -std=c++14 -I/usr/include -I/usr/include/eigen3 -L/usr/lib -L/usr/lib/nvidia-375 -lglut -lGLEW -lGL -lGLU -lX11 -Wno-write-strings
endif

ifeq ($(OS), Darwin)
  FLAGS = -std=c++14 -framework Cocoa -framework OpenGL -framework GLUT -I/usr/local/Cellar/eigen/3.3.4/include/eigen3/ -I/opt/X11/include -L/opt/X11/lib -lGLEW -lGLUT -Wno-write-strings -Wno-deprecated
endif

all:
	g++ modelViewer.cpp -o modelViewer $(FLAGS)
	g++ modelViewerVBO.cpp -o modelViewerVBO $(FLAGS)
	g++ modelViewerVAO.cpp -o modelViewerVAO $(FLAGS)
