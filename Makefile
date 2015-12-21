CXX=distcc arm-linux-gnueabihf-g++ -std=gnu++0x $(INCLUDEFLAGS)
INCLUDEFLAGS=-I/opt/vc/include -I/opt/vc/include/interface/vmcs_host/linux -I/opt/vc/include/interface/vcos/pthreads -I.. -I/home/pi/openvg/client/config4cpp/include
LIBFLAGS=-L/opt/vc/lib -L/opt/vc/lib -L/home/pi/openvg/client/config4cpp/lib -lGLESv2 -lEGL -lbcm_host -lpthread  -ljpeg
objects = serial.o touchscreen.o Gauge.o TouchableObject.o DataStream.o Button.o TextView.o parsingUtilities.o Project.o

all: $(objects) Project

Project.o: Project.cpp

TextView.o: TextView.cpp
Button.o: Button.cpp
DataStream.o: DataStream.cpp
Gauge.o: Gauge.cpp
serial.o: serial.cpp
TouchableObject.o: TouchableObject.cpp
touchscreen.o: touchscreen.cpp
parsingUtilities.o: parsingUtilities.cpp


Project: Project $(objects)
	g++ -Wall -std=gnu++0x $(LIBFLAGS) -o Project $(objects) ../libshapes.o ../oglinit.o -lbcm2835 -lconfig4cpp
	rm *.o
	rm /tmp/distcc*.*
