INCLUDEFLAGS=-I/opt/vc/include -I/opt/vc/include/interface/vmcs_host/linux -I/opt/vc/include/interface/vcos/pthreads -I..
LIBFLAGS=-L/opt/vc/lib -lGLESv2 -lEGL -lbcm_host -lpthread  -ljpeg 

all: Project

Project: Project.cpp Gauge.cpp DataStream.cpp serial.cpp touchscreen.cpp ../libshapes.o ../oglinit.o
	g++ -std=gnu++0x -Wall $(INCLUDEFLAGS) $(LIBFLAGS) -o Project Project.cpp Gauge.cpp DataStream.cpp serial.cpp touchscreen.cpp ../libshapes.o ../oglinit.o -lbcm2835


