using namespace std;
#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include "VG/openvg.h"	//
#include "VG/vgu.h"	//
#include "fontinfo.h"	// Openvg
#include "shapes.h"	//
#include <stdlib.h>	//
#include "serial.h"	// Serial functions
#include "Gauge.h"	// Gauge class
#include "DataStream.h"	// DataStream class
#include <linux/input.h>
#include <fcntl.h>
#include <pthread.h>
#include "avengeance.inc"	// *** Label Fonts
#include "digits.inc"		// *** Label Fonts
#include <bcm2835.h>
#include "touchscreen.h"
#include "project.h"
#include "Button.h"
#include "TextView.h"
#include "Menu.h"
#include "Serial.h"
#include <vector>

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




// main()
int main()
{
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

	Menu Mode1Menu(width/2, height-40, width-20, 60, "Mode1Menu");
	Mode1Menu.touchEnable();
	//Mode1Menu.selectButton("m1");

	Menu TestCommandMenu(width-width/4, height/2-40, width/2-20, height-80, "TestCommandMenu");
	TestCommandMenu.touchEnable();

	Serial ELMSerial("/dev/ELM327", 5);
	ELMSerial.serialWrite("ATZ");

	TextView SerialViewer(width/4, height/2-40, width/2-20, height-80, "SerialViewer");


	while(1) {
		loopTime = bcm2835_st_read();
		char serialData[256];
		readSerial(uart0_filestream, serialData);			// Capture serial data
		BoostDataStream.update(serialData, loopTime);		// Update datastream with serial data

		// Grab touch data at the begining of each loop and 
		loopTouch = threadTouch;
		// Draw background image
		vgSetPixels(0, 0, BackgroundImage, 0, 0, 800, 480);

		Mode1Menu.update(loopTouch);
		TestCommandMenu.update(loopTouch);

		string testCommand = TestCommandMenu.getPressedButtonName();
		if(!testCommand.empty()){
			SerialViewer.addNewLine(testCommand, sendcolor);
			ELMSerial.serialWrite(testCommand);
			TestCommandMenu.selectButton(testCommand);
		}


		SerialViewer.update();
		

		string data = ELMSerial.serialRead();

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
			SerialViewer.addNewLine("ATZ", sendcolor);
			ELMSerial.serialWrite("ATZ");

		}
		if(Mode1Menu.isButtonPressed("m2")) {
			Mode1Menu.selectButton("m2");
			SerialViewer.addNewLine("ATE0", sendcolor);
			ELMSerial.serialWrite("ATE0");

		}
		if(Mode1Menu.isButtonPressed("m3")) {
			Mode1Menu.selectButton("m3");
			SerialViewer.addNewLine("", sendcolor);
			ELMSerial.serialWrite("\n");

		}
		if(Mode1Menu.isButtonPressed("m4")) {
			Mode1Menu.selectButton("m4");
			SerialViewer.clear();
		}
		if(Mode1Menu.isButtonPressed("m5")) Mode1Menu.selectButton("m5");
		if(Mode1Menu.isButtonPressed("m6")) Mode1Menu.selectButton("m6");

		if(!Mode1Menu.isHidden() && Mode1Menu.isPressedOutside())
			{
				Mode1Menu.hide();
				cout << "trying to hide menu" << endl;
			}
		else if(Mode1Menu.isHidden() && Mode1Menu.isPressedOutside())
			{
				Mode1Menu.unhide();
				cout << "trying to hide menu" << endl;
			}



		End();
	}
}

// setupGraphics()
void setupGraphics(int* widthPtr, int* heightPtr) {	
	init(widthPtr,heightPtr);
	Start(*widthPtr, *heightPtr);		//Set up graphics, start picture
	Background(0,0,0);
}




