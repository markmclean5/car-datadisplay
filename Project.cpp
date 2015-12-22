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

bool menuHidden = false;

//Vector of diplay objects
vector<Gauge> objects;
vector<Button> menu;

int findMenuButton(string ident) {
	int idx = 0;
	for(int i = 0; i<menu.size(); i++) {
		if(ident.compare(menu[i].getIdentifier()) == 0)
			idx =i;
	}
	return idx;
}

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
	Image(0, 0, 800, 480, "background.jpg");
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
	float gaugeColor[] = {0,1,0,1};
	float majorTickColor[] = {0,1,0,1};
	float minorTickColor[] = {1,1,1,1};
	float black[] = {0,0,0,1};
	float green[] = {0,1,0,1};
	float red[] = {1,0,0,1};
	float blue[] = {0,0,1,1};
	float white[] = {1,1,1,1};
	float translucentBlack[] = {0,0,0,0.5};
	float gray[] = {0.43,0.43,0.43,1};
	// Create Boost gauge!!
	Gauge BoostGauge(width/2,height/2,width/6, "Boost Gauge");
	//BoostGauge.configure("Boost Gauge");
	BoostGauge.touchEnable();
	BoostGauge.draw();

	// Menu Button style properties
	int mbWidth = 100;
	int mbHeight = 50;
	int mbPadding = 10;
	int mbPeek = 10;

	// Button 1: Refresh rate button
	Button button1("FramerateButton");
	button1.touchEnable();
	button1.draw();
	// Menu control button
	Button menuControlButton("MenuControlButton");
	string hideText = "HIDE";
	string showText = "MENU";
	string defaultText = " ";
	menuControlButton.draw();
	menuControlButton.touchEnable();

	

	Button exitButton(width-35, height-35, 50, 50, "ExitButton");
	exitButton.draw();
	exitButton.touchEnable();

	TextView textView1(width/6, height/2, width/3-15, width/3-15, "TextView1");
	textView1.update();

	int lineNumber = 1;

	// Menu Buttons

	menu.push_back(Button("MenuButton1"));
	menu.push_back(Button("MenuButton2"));
	menu.push_back(Button("MenuButton3"));
	menu.push_back(Button("MenuButton4"));

	for(int i = 0; i<menu.size(); i++) {
		menu[i].touchEnable();
	}



	while(1)
	{
		loopTime = bcm2835_st_read();
		char serialData[256];
		readSerial(uart0_filestream, serialData);			// Capture serial data
		BoostDataStream.update(serialData, loopTime);		// Update datastream with serial data

		// Grab touch data at the begining of each loop and 
		loopTouch = threadTouch;

		// Draw background image
		vgSetPixels(0, 0, BackgroundImage, 0, 0, 800, 480);
		
		for(int i = 0; i < objects.size();i++) {
			objects[i].update(BoostDataStream.getWeightedMADatum(), BoostDataStream.getEngUnits());
			objects[i].updateTouch(loopTouch);
		}

		button1.setValue(BoostDataStream.getRawUpdateRate());
		button1.update();

		button1.updateTouch(loopTouch);
		if (button1.isTouched())
		{
			cout << "Button 1 was touched!!!" << endl;
		}

		menuControlButton.update();
		menuControlButton.updateTouch(loopTouch);
		
		

		for(int i = 0; i<menu.size(); i++) {
			menu[i].update();
			menu[i].updateTouch(loopTouch);
		}

		if(menu[findMenuButton("MenuButton1")].isPressed()) {
			cout << "MenuButton1 was touched" << endl;
			if(objects.size()==0) {
				cout << "Creating Gauge!" << endl;
				objects.push_back(Gauge(width/2,height/2,width/6, "Boost Gauge"));
				objects[objects.size()-1].draw();
				menu[findMenuButton("MenuButton1")].setText("Remove");
			}
			else {
				objects.erase(objects.end());
				menu[findMenuButton("MenuButton1")].setText("Add");
			}
		}

		if(menu[findMenuButton("MenuButton2")].isPressed())
		{
			textView1.addNewLine("New Line #" + std::to_string(lineNumber));
			lineNumber++;
		}

		exitButton.update();
		exitButton.updateTouch(loopTouch);
		if (exitButton.isPressed())
		{
			cout << "Exit Button was pressed!!!" << endl;
		}

		textView1.update();
		textView1.updateTouch(loopTouch);
		if (textView1.isTouched())
		{
			cout << "Text View 1 was touched!!!" << endl;
		}

		
		if (menuControlButton.isPressed()) {
			if(!menuHidden)	{
				for(int i = 0; i<menu.size(); i++) {
					menu[i].move(0, -50, 600, "AAA");
					menu[i].fade(60,600,"AAA");
				}
				menuHidden = true;
			}
			else {
				for(int i = 0; i<menu.size(); i++) {
					menu[i].move(0, 50, 600, "AAA");
					menu[i].fade(0,600,"AAA");
				}
				menuHidden = false;
			}
		}

		if(menuControlButton.isPressedOutside())
			cout << "Somewhere other than the menu control button was pressed." << endl;

		if(menuHidden) menuControlButton.setText(showText);
		else menuControlButton.setText(hideText);

		if(menu[findMenuButton("MenuButton3")].isPressed())
		{
			textView1.clear();
		}

		if(menu[findMenuButton("MenuButton4")].isPressed())
		{
			textView1.addNewLine("New Blue Line #" + std::to_string(lineNumber), blue);
			lineNumber++;
		}

		End();
	}
}

// setupGraphics()
void setupGraphics(int* widthPtr, int* heightPtr)
{	
	init(widthPtr,heightPtr);
	Start(*widthPtr, *heightPtr);		//Set up graphics, start picture
	Background(0,0,0);
}




