INCLUDEFLAGS=-I/opt/vc/include -I/opt/vc/include/interface/vmcs_host/linux -I/opt/vc/include/interface/vcos/pthreads -I..
LIBFLAGS=-L/opt/vc/lib -lGLESv2 -lEGL -lbcm_host -lpthread  -ljpeg 

all: Project

Project: Project.cpp Gauge.cpp TouchableObject.cpp DataStream.cpp serial.cpp touchscreen.cpp Button.cpp ../libshapes.o ../oglinit.o
	g++ -std=gnu++0x -Wall $(INCLUDEFLAGS) $(LIBFLAGS) -o Project Project.cpp Gauge.cpp TouchableObject.cpp DataStream.cpp serial.cpp touchscreen.cpp Button.cpp ../libshapes.o ../oglinit.o -lbcm2835


