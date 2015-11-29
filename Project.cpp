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

// main()
int main()
{
	int width, height;					// display width & height
	setupGraphics(&width, &height);		// Initialize display
	int uart0_filestream = openSerial();
	if (!bcm2835_init())
<<<<<<< Updated upstream
        	return 1;
=======
		return 1;
>>>>>>> Stashed changes
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
<<<<<<< Updated upstream

=======
>>>>>>> Stashed changes
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
<<<<<<< Updated upstream

=======
	float gray[] = {0.43,0.43,0.43,1};
>>>>>>> Stashed changes
	// Create Boost gauge!!
	Gauge BoostGauge(width/2,height/2,width/6);
	BoostGauge.setNumRanges(2);
	BoostGauge.setBorderColor(gaugeColor);
	BoostGauge.setBackgroundColor(translucentBlack);
	BoostGauge.setNeedleColor(gaugeColor);
	// Configure Boost gauge range 1
	BoostGauge.setDataRange(0,30,1);
	BoostGauge.setDataAngleRange(0,-180,1);
	//BoostGauge.setEngUnits(eu1, 1);
	BoostGauge.setMajorInterval(5,1);
	BoostGauge.setMinorInterval(1,1);
	BoostGauge.setMajorTickColor(majorTickColor, 1);
	BoostGauge.setMinorTickColor(minorTickColor, 1);
	BoostGauge.setLabelColor(gaugeColor, 1);
	BoostGauge.setLabelRange(0, 30, 1);
	BoostGauge.setLabelAngleRange(0, 180, 1);
	BoostGauge.setLabelIncrement(5, 1);
	BoostGauge.setLabelDecPlaces(0, 1);
	BoostGauge.setLabelFont(avengeance, 1);
	// Configure Boost gauge range 2
	BoostGauge.setDataRange(0,30,2);
	BoostGauge.setDataAngleRange(0,90,2);
	//BoostGauge.setEngUnits(eu2, 2);
	BoostGauge.setMajorInterval(10,2);
	BoostGauge.setMinorInterval(2,2);
	BoostGauge.setMajorTickColor(majorTickColor, 2);
	BoostGauge.setMinorTickColor(minorTickColor, 2);
	BoostGauge.setLabelColor(gaugeColor, 2);
	BoostGauge.setLabelRange(10, 30, 2);
	BoostGauge.setLabelAngleRange(-30, -90, 2);
	BoostGauge.setLabelIncrement(10, 2);
	BoostGauge.setLabelDecPlaces(0, 2); 
	BoostGauge.setLabelFont(avengeance, 2);
	BoostGauge.touchEnable();
<<<<<<< Updated upstream
=======

	BoostGauge.configure("Boost Gauge");
>>>>>>> Stashed changes
	BoostGauge.draw();

	// Button 1: Refresh rate button
	Button button1(width/2+115, height/2, 100, 50);
	button1.setBorder(green, 2); 
	button1.enableText('T');
	button1.setTextColor(green);
	string textString = "Hz";
	button1.setText(textString);
	button1.enableValue('B');
	button1.setValueRefreshRate(5); 
	button1.setValueDecPlaces(2);
	button1.setValueColor(red);
	button1.touchEnable();
	button1.draw();

<<<<<<< Updated upstream
	// Exit button
	Button exitButton(width-40, 40, 50, 50);
	exitButton.setBorder(red, 2);
	exitButton.enableText('C');
	exitButton.setTextColor(red);
	string hideText = "X";
	string showText = "-";
	string transitionText = " ";
	exitButton.setText(hideText);
	exitButton.setPressDebounce(500);
	exitButton.touchEnable();
	exitButton.draw();



	int mbWidth = 100;
	int mbHeight = 50;
	int mbPadding = 10;
=======

	// Menu Button style properties
	int mbWidth = 100;
	int mbHeight = 50;
	int mbPadding = 10;
	int mbPeek = 10;
	// Menu control button
	Button menuControlButton(width-mbWidth/2-mbPadding, mbPadding+mbHeight/2, mbWidth, mbHeight);
	menuControlButton.setCornerRadius(20);
	menuControlButton.setBorder(gray, 2);
	menuControlButton.enableText('C');
	menuControlButton.setTextColor(gray);
	string hideText = "HIDE";
	string showText = "MENU";
	string defaultText = " ";
	menuControlButton.setText(defaultText);
	menuControlButton.setPressDebounce(500);
	menuControlButton.touchEnable();
	menuControlButton.draw();

>>>>>>> Stashed changes
	// Menu Buttons
	Button menuButton1(mbPadding+mbWidth/2,mbPadding+mbHeight/2, mbWidth, mbHeight);
	menuButton1.setCornerRadius(20);
	menuButton1.setBorder(white,2);
	menuButton1.enableText('C');
	menuButton1.setTextColor(white);
	string menuButton1Text = "Menu 1";
	menuButton1.setText(menuButton1Text);
	menuButton1.setPressDebounce(500);
	menuButton1.draw();
	menuButton1.touchEnable();


	Button menuButton2(mbWidth+2*mbPadding+mbWidth/2,mbPadding+mbHeight/2, mbWidth, mbHeight);
	menuButton2.setCornerRadius(20);
	menuButton2.setBorder(white,2);
	menuButton2.enableText('C');
	menuButton2.setTextColor(white);
	string menuButton2Text = "Menu 2";
	menuButton2.setText(menuButton2Text);
	menuButton2.draw();
	menuButton2.touchEnable();

	Button menuButton3(2*mbWidth+3*mbPadding+mbWidth/2,mbPadding+mbHeight/2, mbWidth, mbHeight);
	menuButton3.setCornerRadius(20);
	menuButton3.setBorder(white,2);
	menuButton3.enableText('C');
	menuButton3.setTextColor(white);
	string menuButton3Text = "Menu 3";
	menuButton3.setText(menuButton3Text);
	menuButton3.draw();
	menuButton3.touchEnable();

	Button menuButton4(3*mbWidth+4*mbPadding+mbWidth/2,mbPadding+mbHeight/2, mbWidth, mbHeight);

	menuButton4.setCornerRadius(20);
	menuButton4.setBorder(white,2);
	menuButton4.enableText('C');
	menuButton4.setTextColor(white);
	string menuButton4Text = "Menu 4";
	menuButton4.setText(menuButton4Text);
	menuButton4.draw();
	menuButton4.touchEnable();
	while(1)
	{
		loopTime = bcm2835_st_read();
		char serialData[256];
		readSerial(uart0_filestream, serialData);			// Capture serial data
		BoostDataStream.update(serialData, loopTime);		// Update datastream with serial data
<<<<<<< Updated upstream

		// Grab touch data at the begining of each loop and 
		loopTouch = threadTouch;

		vgSetPixels(0, 0, BackgroundImage, 0, 0, 800, 480);


		BoostGauge.update(BoostDataStream.getWeightedMADatum(), BoostDataStream.getEngUnits());
=======

		// Grab touch data at the begining of each loop and 
		loopTouch = threadTouch;

		vgSetPixels(0, 0, BackgroundImage, 0, 0, 800, 480);

		BoostGauge.update(BoostDataStream.getWeightedMADatum(), BoostDataStream.getEngUnits());
		BoostGauge.updateVisuals();
>>>>>>> Stashed changes
		BoostGauge.updateTouch(loopTouch);
		if (BoostGauge.isTouched()) cout << "Boost Gauge was touched!!!" << endl;
		
		button1.setValue(BoostDataStream.getRawUpdateRate());
		button1.update();
		button1.updateVisuals();

		button1.updateTouch(loopTouch);
		if (button1.isTouched())
		{
			cout << "Button 1 was touched!!!" << endl;
		}

<<<<<<< Updated upstream
		exitButton.update();
		exitButton.updateVisuals();
		exitButton.updateTouch(loopTouch);
=======
		menuControlButton.update();
		menuControlButton.updateVisuals();
		menuControlButton.updateTouch(loopTouch);
>>>>>>> Stashed changes
		
		menuButton1.update();
		menuButton1.updateVisuals();
		menuButton1.updateTouch(loopTouch);
		if (menuButton1.isTouched())
		{
			cout << "Menu Button 1 was touched!!!" << endl;
			menuButton1.fade(50, 1000, "AAA");
		}

		menuButton2.update();
		menuButton2.updateVisuals();
		menuButton2.updateTouch(loopTouch);
		if (menuButton2.isTouched())
		{
			cout << "Menu Button 2 was touched!!!" << endl;
<<<<<<< Updated upstream
=======
			BoostGauge.fade(100, 2000, "AAA");
>>>>>>> Stashed changes
		}

		menuButton3.update();
		menuButton3.updateVisuals();
		menuButton3.updateTouch(loopTouch);
		if (menuButton3.isTouched())
		{
			cout << "Menu Button 3 was touched!!!" << endl;
		}

		menuButton4.update();
		menuButton4.updateVisuals();
		menuButton4.updateTouch(loopTouch);
		if (menuButton4.isTouched())
		{
			cout << "Menu Button 4 was touched!!!" << endl;
		}

<<<<<<< Updated upstream
		if (exitButton.isPressed())
=======
		if (menuControlButton.isPressed())
>>>>>>> Stashed changes
		{
			cout << "Exit Button was touched!!!" << endl;
			if(!menuHidden)
			{
<<<<<<< Updated upstream
				menuButton1.move(mbPadding+mbWidth/2,0-mbHeight/2, 1000, "AAA");
				menuButton2.move(mbWidth+2*mbPadding+mbWidth/2, 0-mbHeight/2, 1000, "AAA");
				menuButton3.move(2*mbWidth+3*mbPadding+mbWidth/2, 0-mbHeight/2, 1000, "AAA");
				menuButton4.move(3*mbWidth+4*mbPadding+mbWidth/2, 0-mbHeight/2, 1000, "AAA");
=======
				menuButton1.move(mbPadding+mbWidth/2,0-mbHeight/2+mbPeek, 600, "AAA");
				menuButton1.fade(60,600,"AAA");
				menuButton2.move(mbWidth+2*mbPadding+mbWidth/2, 0-mbHeight/2+mbPeek, 600, "AAA");
				menuButton2.fade(60,600,"AAA");
				menuButton3.move(2*mbWidth+3*mbPadding+mbWidth/2, 0-mbHeight/2+mbPeek, 600, "AAA");
				menuButton3.fade(60,600,"AAA");
				menuButton4.move(3*mbWidth+4*mbPadding+mbWidth/2, 0-mbHeight/2+mbPeek, 600, "AAA");
				menuButton4.fade(60,600,"AAA");
>>>>>>> Stashed changes
				menuHidden = true;

			}
			else
			{
<<<<<<< Updated upstream
				menuButton1.move(mbPadding+mbWidth/2,mbPadding+mbHeight/2, 1000, "AAA");
				menuButton2.move(mbWidth+2*mbPadding+mbWidth/2, mbPadding+mbHeight/2, 1000, "AAA");
				menuButton3.move(2*mbWidth+3*mbPadding+mbWidth/2, mbPadding+mbHeight/2, 1000, "AAA");
				menuButton4.move(3*mbWidth+4*mbPadding+mbWidth/2, mbPadding+mbHeight/2, 1000, "AAA");
				menuHidden = false;
			}
		}

=======
				menuButton1.move(mbPadding+mbWidth/2,mbPadding+mbHeight/2, 600, "AAA");
				menuButton1.fade(0,600,"AAA");
				menuButton2.move(mbWidth+2*mbPadding+mbWidth/2, mbPadding+mbHeight/2, 600, "AAA");
				menuButton2.fade(0,600,"AAA");
				menuButton3.move(2*mbWidth+3*mbPadding+mbWidth/2, mbPadding+mbHeight/2, 600, "AAA");
				menuButton3.fade(0,600,"AAA");
				menuButton4.move(3*mbWidth+4*mbPadding+mbWidth/2, mbPadding+mbHeight/2, 600, "AAA");
				menuButton4.fade(0,600,"AAA");
				menuHidden = false;

			}
		}
		if(menuHidden) menuControlButton.setText(showText);
		else menuControlButton.setText(hideText);
>>>>>>> Stashed changes
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




