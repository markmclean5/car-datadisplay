using namespace std;
#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include "VG/openvg.h"	//
#include "VG/vgu.h"		//
#include "EGL/egl.h"	// EGL for pbuffers
#include "fontinfo.h"	// Openvg
#include "shapes.h"		//
#include <stdlib.h>		//
#include "Gauge.h"	// Gauge class
#include "DataStream.h"	// DataStream class
#include <linux/input.h>
#include <fcntl.h>
#include <pthread.h>
#include "avengeance.inc"	// *** Label Fonts
#include "digits.inc"		// *** Label Fonts
#include <bcm2835.h>
#include <vector>

// Project Classes & utilities
#include "touchscreen.h"
#include "project.h"
#include "Button.h"
#include "TextView.h"
#include "Menu.h"
#include "Serial.h"
#include "PID.h"


// Loop time
uint64_t loopTime;
touch_t threadTouch, loopTouch;
int quitState = 0;

VGImage BackgroundImage;

// Prototypes
void setupGraphics(int*,int*);

//Vector of diplay objects
vector<TouchableObject> TObjects;


// Color definitions (float r, float g, float b, float alpha)
float black[] = {0,0,0,1};
float green[] = {0,1,0,1};
float red[] = {1,0,0,1};
float blue[] = {0,0,1,1};
float white[] = {1,1,1,1};
float translucentBlack[] = {0,0,0,0.5};
float gray[] = {0.43,0.43,0.43,1};
float sendcolor[] = {1.0, 0.4, 0.4, 1.0};
float receivecolor[] = {0.4, 0.69, 1.0, 1.0};

// main()
int main() {
	int width, height;					// display width & height
	setupGraphics(&width, &height);		// Initialize display

	if (!bcm2835_init())
		return 1;
	if (touchinit(width, height) != 0) {
		fprintf(stderr, "Unable to initialize the mouse\n");
		exit(1);
	}

	// Draw background
	Image(0, 0, 800, 480, "/home/pi/openvg/client/wallpaper.jpg");
	BackgroundImage = vgCreateImage(VG_sABGR_8888, 800, 480, VG_IMAGE_QUALITY_BETTER);
	vgGetPixels(BackgroundImage, 0, 0, 0, 0, 800, 480);
	

	Menu Mode1Menu(width/2, height-30, width-20, 50, "Mode1Menu");
	Mode1Menu.touchEnable();
	Mode1Menu.selectButton("m1");
	int currentMode = 0;

	Serial ELMSerial("/dev/ELM327", B38400);
	ELMSerial.serialWrite("ATZ");
	ELMSerial.setEndChar('>');
	ELMSerial.setReadTimeout(5000);

	PID TestPID("0105");


	while(1) {
		loopTime = bcm2835_st_read();
		// Grab touch data at the begining of each loop and 
		loopTouch = threadTouch;
		// Draw background image
		vgSetPixels(0, 0, BackgroundImage, 0, 0, 800, 480);
		
		// Update menus
		Mode1Menu.update(loopTouch);


		if(Mode1Menu.isButtonPressed("m1")) Mode1Menu.selectButton("m1");
		if(Mode1Menu.isButtonPressed("m2")) Mode1Menu.selectButton("m2");
		if(Mode1Menu.isButtonPressed("m3")) Mode1Menu.selectButton("m3");
		if(Mode1Menu.isButtonPressed("m4")) Mode1Menu.selectButton("m4");
		if(Mode1Menu.isButtonPressed("m5")) Mode1Menu.selectButton("m5");
		if(Mode1Menu.isButtonPressed("m6")) Mode1Menu.selectButton("m6");


		if(Mode1Menu.isButtonSelected("m6")) {
			TextView SerialViewer(width/4, height/2, width/2-20, 360, "SerialViewer");
			Menu SerialViewerMenu(width/4, 30, width/2-20, 50, "SerialViewerMenu");
			Menu PIDMenu(width-width/4, height/2, width/2-20, 360, "PIDMenu1");
			
			SerialViewerMenu.touchEnable();
			PIDMenu.touchEnable();

			while(1) {
				loopTime = bcm2835_st_read();
				loopTouch = threadTouch;
				vgSetPixels(0, 0, BackgroundImage, 0, 0, 800, 480);
				
				TestPID.update("010541 05 26", loopTime);
				cout << "TestPID value" << TestPID.getRawDatum() << endl;



				Mode1Menu.update(loopTouch);
				SerialViewerMenu.update(loopTouch);
				PIDMenu.update(loopTouch);

				string serialViewerCommand = SerialViewerMenu.getPressedButtonName();
				if(!serialViewerCommand.empty()) {
					SerialViewerMenu.selectButton(serialViewerCommand);
					if(serialViewerCommand.compare("Reset") == 0) {
						SerialViewer.addNewLine("ATZ", sendcolor);
						ELMSerial.serialWrite("ATZ");
					}
					if(serialViewerCommand.compare("Auto") == 0) {
						SerialViewer.addNewLine("ATSP0", sendcolor);
						ELMSerial.serialWrite("ATSP0");
					}
					if(serialViewerCommand.compare("Disp") == 0) {
						SerialViewer.addNewLine("ATDP", sendcolor);
						ELMSerial.serialWrite("ATDP");
					}
					if(serialViewerCommand.compare("Clear") == 0) {
						SerialViewer.clear();
					}
				}


				string commandString = PIDMenu.getPressedButtonName();
				if(!commandString.empty()) {
					PIDMenu.selectButton(commandString);
					SerialViewer.addNewLine(commandString, sendcolor);
					ELMSerial.serialWrite(commandString);
				}
				
				SerialViewer.update();
				string data = ELMSerial.serialReadUntil();


				if(!data.empty()) {
					cout << "Data: " << endl << data << endl;
					cout << "Data characters: " << endl;
					for(int idx=0; idx<data.size(); idx++)
						cout << (int)data[idx] << " ";
					cout << endl;
					SerialViewer.addNewLine(data, receivecolor);
				}
				if(Mode1Menu.isButtonPressed("m1")) {
					Mode1Menu.selectButton("m1");
					break;
				}
				if(Mode1Menu.isButtonPressed("m2")) {
					Mode1Menu.selectButton("m2");
					break;
				}
				if(Mode1Menu.isButtonPressed("m3")) {
					Mode1Menu.selectButton("m3");
					break;
				}
				if(Mode1Menu.isButtonPressed("m4")) {
					Mode1Menu.selectButton("m4");
					break;
				}
				if(Mode1Menu.isButtonPressed("m5")) {
					Mode1Menu.selectButton("m5");
					break;
				}
				End();
			}

		}
		// Write screen buffer to screen
		End();
	}
}

// setupGraphics()
void setupGraphics(int* widthPtr, int* heightPtr) {	
	init(widthPtr,heightPtr);
	Start(*widthPtr, *heightPtr);		//Set up graphics, start picture
	Background(0,0,0);
}




EGLSurface createBufferSurface(int surfaceWidth, int surfaceHeight, VGImage* colorBuffer) {


}