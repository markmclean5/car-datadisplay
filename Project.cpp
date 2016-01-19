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
#include "serial.h"	// Serial functions
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


// Label and readout fonts, loop time
Fontinfo avengeance;
Fontinfo digits;
uint64_t loopTime;
touch_t threadTouch, loopTouch;
int quitState = 0;

VGImage BackgroundImage;

// Prototypes
void setupGraphics(int*,int*);

//Vector of diplay objects
vector<TouchableObject> TObjects;

float sendcolor[] = {1.0, 0.4, 0.4, 1.0};
float colour[4];


// main()
int main() {
	int width, height;					// display width & height
	setupGraphics(&width, &height);		// Initialize display
	int uart0_filestream = openSerial();
	if (!bcm2835_init())
		return 1;
	if (touchinit(width, height) != 0) {
		fprintf(stderr, "Unable to initialize the mouse\n");
		exit(1);
	}
	// Label and readout fonts
	avengeance = loadfont(avengeance_glyphPoints, 
		avengeance_glyphPointIndices, 
		avengeance_glyphInstructions,                
		avengeance_glyphInstructionIndices, 
		avengeance_glyphInstructionCounts, 
		avengeance_glyphAdvances,
		avengeance_characterMap, 
		avengeance_glyphCount);
	digits = loadfont(digits_glyphPoints, 
		digits_glyphPointIndices, 
		digits_glyphInstructions,                
		digits_glyphInstructionIndices, 
		digits_glyphInstructionCounts, 
		digits_glyphAdvances,
		digits_characterMap, 
		digits_glyphCount);
	// Draw background
	Image(0, 0, 800, 480, "wallpaper.jpg");
	BackgroundImage = vgCreateImage(VG_sABGR_8888, 800, 480, VG_IMAGE_QUALITY_BETTER);
	vgGetPixels(BackgroundImage, 0, 0, 0, 0, 800, 480);
	DataStream BoostDataStream(2);
	BoostDataStream.setDebugMode(false);
	BoostDataStream.setCharTags('A', 'B');
	BoostDataStream.setStreamScaling(100.);
	BoostDataStream.setRangeScaling(1., 1);
	BoostDataStream.setRangeLimits(0.,30.,1);
	BoostDataStream.setWeightedMALag(10,1);
	BoostDataStream.setSimpleMALag(3,1);
	BoostDataStream.setRangeScaling(-2.036, 2);
	BoostDataStream.setRangeLimits(0,-14.734,2);	// Define range with respect to serial stream input units 
	BoostDataStream.setWeightedMALag(10,2);
	BoostDataStream.setSimpleMALag(3,2);
	float coeffs[] = {0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1};
	BoostDataStream.setWeightedMACoeffs(coeffs,1);
	BoostDataStream.setWeightedMACoeffs(coeffs,2);
	string eu1 = "PSI";
	BoostDataStream.setEngUnits(eu1, 1);
	string eu2 = "inHg";
	BoostDataStream.setEngUnits(eu2, 2);
	// Color definitions (float r, float g, float b, float alpha)
	float black[] = {0,0,0,1};
	float green[] = {0,1,0,1};
	float red[] = {1,0,0,1};
	float blue[] = {0,0,1,1};
	float white[] = {1,1,1,1};
	float translucentBlack[] = {0,0,0,0.5};
	float gray[] = {0.43,0.43,0.43,1};

	TextView textView1(width/6, height/2, width/3-15, width/3-15, "TextView1");
	textView1.update();

	bool menuActive = false;

	Menu Mode1Menu(width/2, height-30, width-20, 50, "Mode1Menu");
	Mode1Menu.touchEnable();
	Mode1Menu.selectButton("m1");
	int currentMode = 0;

	Serial ELMSerial("/dev/ELM327", B38400);
	ELMSerial.serialWrite("ATZ");
	ELMSerial.setEndChar('>');
	ELMSerial.setReadTimeout(5000);

	int currentPIDMenuPage = 0;
	int desiredPIDMenuPage = 1;
	int numPages = 3;

	vector<Menu> PIDMenus;
	string menuPrefix = "PIDMenu";

	// Off screen buffer hijinks

	float color1;


	VGImage myImage = vgCreateImage(VG_sABGR_8888, 800, 480, VG_IMAGE_QUALITY_BETTER);

	static const EGLint attribute_list[] = {
		EGL_RED_SIZE, 8,
		EGL_GREEN_SIZE, 8,
		EGL_BLUE_SIZE, 8,
		EGL_ALPHA_SIZE, 8,
		EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
		EGL_NONE
	};

	EGLint num_config;
	EGLBoolean result;
	EGLConfig config;

	EGLDisplay realDisplay = eglGetCurrentDisplay();
	if(realDisplay == EGL_NO_DISPLAY)
	{
		cout << "Failed to get current display" << endl;
	}


	result = eglChooseConfig(realDisplay, attribute_list, &config, 1, &num_config);
	
	//result = eglGetConfigs(realDisplay, config, 1, 1);
	if(result == EGL_FALSE) {
		cout << "Failed to choose config" << endl;
	}


	EGLSurface realSurface = eglGetCurrentSurface(EGL_DRAW);

	if(realSurface == EGL_NO_SURFACE) {
		cout << "Failed to get current surface" << endl;
	}

	EGLContext realContext = eglGetCurrentContext();

	if(realContext == EGL_NO_CONTEXT) {
		cout << "Failed to get current context" << endl;
	}

	static const EGLint surfAttr[] = {
		EGL_HEIGHT, 480,
		EGL_WIDTH, 800,
		EGL_NONE
	};

	EGLSurface mySurface = eglCreatePbufferFromClientBuffer (realDisplay, EGL_OPENVG_IMAGE, (EGLClientBuffer)myImage, config, surfAttr);


	if(mySurface == EGL_NO_SURFACE) {
		cout << "Failed to create pbuffer surface" << endl;
	}
	result = eglMakeCurrent(realDisplay, mySurface, mySurface, realContext);
	if(result == EGL_FALSE)
	{
		cout << "Failed to make new display current" << endl;
	}



	


	//int broken = 1;
	int morebroken = 17;
	//float borked = 5;

	RGBA(0, 0, 0, 0.1, colour);
	vgSetfv(VG_CLEAR_COLOR, 4, colour);
	vgClear(0, 0, 800, 480);

	Stroke(255,255,255,1);
	StrokeWidth(10);
	Line(width/2-100, height/2, width/2+100, height/2);


	eglMakeCurrent(realDisplay, realSurface, realSurface, realContext);
	if(result == EGL_FALSE)
	{
		cout << "Failed to make original display current" << endl;
	}

	//End();

	//

	while(1) {
		loopTime = bcm2835_st_read();
		char serialData[256];
		readSerial(uart0_filestream, serialData);			// Capture serial data
		BoostDataStream.update(serialData, loopTime);		// Update datastream with serial data

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
			SerialViewerMenu.touchEnable();
			Menu PIDPageMenu(width-width/4, 30, width/2-20, 50, "PIDPageMenu");
			PIDPageMenu.touchEnable();
			while(1) {
				loopTime = bcm2835_st_read();
				loopTouch = threadTouch;
				char mode6serialData[256];
				readSerial(uart0_filestream, mode6serialData);			// Capture serial data
				BoostDataStream.update(mode6serialData, loopTime);		// Update datastream with serial data
				vgSetPixels(0, 0, BackgroundImage, 0, 0, 800, 480);
				//vgSeti(VG_IMAGE_MODE, VG_DRAW_IMAGE_MULTIPLY);
				


				// Draws image saved in buffer:
				// Needed to use vgDrawImage to get opacity to work
				// Needed to set VG_MATRIX_MODE to apply translations to image matrices
				vgSeti(VG_MATRIX_MODE, VG_MATRIX_IMAGE_USER_TO_SURFACE);
				vgTranslate(400, 0);
				vgDrawImage(myImage);
				vgTranslate(-400, 0);
				vgSeti(VG_MATRIX_MODE, VG_MATRIX_PATH_USER_TO_SURFACE);

				
				Mode1Menu.update(loopTouch);
				SerialViewerMenu.update(loopTouch);
				PIDPageMenu.update(loopTouch);
				for(int i = 0; i<PIDMenus.size(); i++) {
					PIDMenus[i].update(loopTouch);
				}
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

				string PIDPageMenuCommand = PIDPageMenu.getPressedButtonName();
				if(!PIDPageMenuCommand.empty()) {
					if(PIDPageMenuCommand.compare("Next") == 0 && (currentPIDMenuPage < numPages)) {
						PIDPageMenu.selectButton(PIDPageMenuCommand);
						desiredPIDMenuPage++;
					}
					if(PIDPageMenuCommand.compare("Prev") == 0 && (currentPIDMenuPage > 1)) {
						PIDPageMenu.selectButton(PIDPageMenuCommand);
						desiredPIDMenuPage--;
					}
				}

				if(currentPIDMenuPage != desiredPIDMenuPage) {
					if(PIDMenus.size()!=0) PIDMenus.erase(PIDMenus.begin());
					PIDMenus.push_back(Menu(width-width/4, height/2, width/2-20, 360, menuPrefix + std::to_string(desiredPIDMenuPage)));
					PIDMenus[0].touchEnable();
					currentPIDMenuPage = desiredPIDMenuPage;
				}
				string commandString = PIDMenus[PIDMenus.size()-1].getPressedButtonName();
				if(!commandString.empty()) {
					PIDMenus[PIDMenus.size()-1].selectButton(commandString);
					SerialViewer.addNewLine(commandString, sendcolor);
					ELMSerial.serialWrite(commandString);
				}

				// Display refresh rate in SerialViewer
				SerialViewer.addNewLine(std::to_string(BoostDataStream.getReadoutUpdateRate()));

				
				SerialViewer.update();
				string data = ELMSerial.serialReadUntil();

				float receivecolor[] = {0.4, 0.69, 1.0, 1.0};
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




