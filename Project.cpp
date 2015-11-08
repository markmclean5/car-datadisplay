using namespace std;

#include <iostream>
#include <stdio.h>
#include <unistd.h>


#include "VG/openvg.h"		//
#include "VG/vgu.h"			//
#include "fontinfo.h"		// OpenVG
#include "shapes.h"			//
#include <stdlib.h>			//

#include "serial.h"			// Serial functions
#include "Gauge.h"			// Gauge class
#include "DataStream.h"		// DataStream class


#include <linux/input.h>
#include <fcntl.h>
#include <pthread.h>


// *** Label Fonts
#include "avengeance.inc"
#include "digits.inc"
#include <bcm2835.h> 

#include "touchscreen.h"
#include "project.h"
#include "Button.h"


// Label and readout fonts, loop time
Fontinfo avengeance;
Fontinfo digits;
uint64_t loopTime;
touch_t touch;
int quitState = 0;

// Prototypes
void setupGraphics(int*,int*);

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

	// create datastream object!!
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

	// Create Boost gauge!!
	Gauge BoostGauge(width/2,height/2,height/2, 2);
	BoostGauge.setBorderColor(gaugeColor);
	BoostGauge.setBackgroundColor(black);
	BoostGauge.setNeedleColor(gaugeColor);

	// Configure Boost gauge range 1
	BoostGauge.setDataRange(0,30,1);
	BoostGauge.setDataAngleRange(0,-180,1);
	BoostGauge.setEngUnits(eu1, 1);
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
	BoostGauge.setEngUnits(eu2, 2);
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

	BoostGauge.draw();

	Button button1(width-100, height/2, 100, 50);
	button1.setBorder(green, 2); 
	//readout1.setDecPlaces(2);
	//readout1.setRefreshRate(5);
	button1.enableText('T');

	button1.setValueRefreshRate(5); 
	button1.enableValue('B');
	button1.setValueDecPlaces(2);
	button1.setTextColor(green);
	button1.setValueColor(red);
	string textString = "Hz";
	button1.setText(textString);
	button1.draw();

	End();						   // End the picture
	
	int last_touch_id = 0;
	while(1)
	{
		loopTime = bcm2835_st_read();
		char serialData[256];
		readSerial(uart0_filestream, serialData);			// Capture serial data
		BoostDataStream.update(serialData, loopTime);
		BoostGauge.update(BoostDataStream.getWeightedMADatum(), BoostDataStream.getEngUnits());
		//cout << "DataStream output data: " << BoostDataStream.getWeightedMADatum() << endl;
		//cout << "DataStream output units: " << BoostDataStream.getEngUnits() << endl;
		//cout << "DataStream update rate: " << BoostDataStream.getRawUpdateRate() << " Hz" << endl;

		button1.setValue(BoostDataStream.getRawUpdateRate());
		button1.update();
		BoostGauge.updateTouch(touch);
		if (BoostGauge.isTouched()) cout << "Boost Gauge was touched!!!" << endl;

		/*
		if(touch.btn_touch == 1 && touch.mt_tracking_id != last_touch_id)
		{
			last_touch_id = touch.mt_tracking_id;
			cout << "New Touch: Main Loop - Touch X: " << touch.abs_x << " Touch Y: " << touch.abs_y << endl;
		}
		else if (touch.btn_touch == 1 && touch.mt_tracking_id == last_touch_id)
		{
			cout << "Touch Held: Main Loop - Touch X: " << touch.abs_x << " Touch Y: " << touch.abs_y << endl;
		}
		else if (touch.btn_touch == 0 && last_touch_id != 0)
		{
			cout << "Touch Released" << endl;
			last_touch_id = 0;
		}
		*/
			
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




