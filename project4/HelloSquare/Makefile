
OS := $(shell uname -s)

ifeq ($(OS), Linux)
  FLAGS = -I/usr/include -I/usr/include/eigen3 -L/usr/lib -L/usr/lib/nvidia-375 -lglut -lGLEW -lGL -lGLU -lX11 -Wno-write-strings
endif

ifeq ($(OS), Darwin)
  FLAGS = -framework OpenGL -framework GLUT -I/opt/X11/include -I/usr/local/include -I/usr/local/include/eigen3 -L/opt/X11/lib -L/usr/local/lib -lGLEW -lGLUT -Wno-write-strings
endif

%: %.cpp
	g++ $*.cpp -o $* $(FLAGS)

