#ifndef GAUGE_H
#define GAUGE_H

#include "TouchableObject.h"

class Gauge: public TouchableObject{
private:
	// Gauge itself
	VGImage GaugeBuffer;

	int centerX;
	int centerY;
	int radius;
	float gaugeRadius;
	float dynamicContentRadius; // Allowable area for dynamic content (refresh )

	float borderColor[4];
	float backgroundColor[4];
	float needleColor[4];

	// Display gauge properties (draw function)
	int numRanges;					// Number of gauge display ranges (currently supports 1 - 5)
	std::string* EngUnits;
	float* startVal;
	float* stopVal;
	float* startAng;
	float* stopAng;
	float* majorInt;
 	float* minorInt;
	float* majorTickColorRed;
	float* majorTickColorGreen;
	float* majorTickColorBlue;
	float* majorTickColorAlpha;
	float* minorTickColorRed;
	float* minorTickColorGreen;
	float* minorTickColorBlue;
	float* minorTickColorAlpha;

	// Label properties 
	float* labelStartVal;
	float* labelStopVal;
	float* labelIncrement;
	int* labelDecPlaces;
	float* labelStartAng;
	float* labelStopAng;
	float* labelColorRed;
	float* labelColorGreen;
	float* labelColorBlue;
	float* labelColorAlpha;
	Fontinfo* labelFont;

public:
	
	// Gauge itself
	Gauge(int, int, int);							// Gauge constructor		(centerX, centerY, radius)
	void setNumRanges(int);
	void setEngUnits(std::string, int);				// Set engineering units string	(Eng Units string, range#)

 	// Draw function setters (tick marks & border)
	void setBorderColor(float*);				// Set border color 			(rgba)
	void setBackgroundColor(float*);			// Set background color 		(rgba)
	void setDataRange(float,float, int);		// Set display data range 		(min, max, range #)
	void setDataAngleRange(float, float, int);	// Set display angular range 	(min, max, range #)
	void setMajorInterval(float, int);			// Set major tick interval 		(interval, range #)
	void setMinorInterval(float, int);			// Set minor tick interval 		(interval, range #)
	void setMajorTickColor(float*, int);		// Set major tick color 		(rgba, range #)
	void setMinorTickColor(float*, int);		// Set major tick color 		(rgba, range #)

	// Draw function
	// (Call after setting all of the above!)
	void draw(void);							// Draw gauge (initialize)

	// Update function setters (labels, needle, etc)
	void setNeedleColor(float*);				// Set needle color 			(rgba)
	void setLabelColor(float*, int);			// Set label label color 		(rgba, range #)
	void setLabelRange(float, float, int);		// Set label display range 		(min, max, range #)
	void setLabelAngleRange(float, float, int);	// Set label angle range 		(min, max, range #)
	void setLabelIncrement(float, int);			// Set label increment 			(increment, range #)
	void setLabelDecPlaces(int, int);			// Set label # of dec places 	(# of decimal places, range #)
	void setLabelFont(Fontinfo, int);			// Set label font				(font)



	void setReadoutColor(float*);				// Set readout color 			(rgba)
	void setReadoutDigits(int);					// Set readout # digits 		(# digits)
	void setReadoutDecPlaces(int);				// Set readout # dec places 	(# of decimal places)
	void setReadoutAngle(float);				// Set readout angle 			(readout center angle)
	void setReadoutRadius(float);				// Set readout radius 			(readout center radius)
	void setReadoutFont(Fontinfo);				// Set label font 				(font)


	void configure(string);						// Auto configure using configGauges file
	// Update function
	// (Call after setting all of the above!)
	void update(float, std::string);			// Update gauge with value		(value, engineering units string)

private:
	// Internally accessed methods (called within draw / update functions)
	void drawTickSet(float, float, float, float, float*, bool);			// Draw tick set(s): 
																		//		(start angle
																		//		 stop angle
																		//		 angle interval
																		//		 angle ratio
																		//		 tick color rgba
																		//		 isMajor)

	void drawLabelSet(float, float, float, int, float, float, float*, Fontinfo);	// Draw label set
																		//		(start label,
																		//		 stop label,
																		//		 label increment,
																		//		 labeldecimal places, 
																		//		 label start angle,
																		//		 label stopAngle,
																		//		 labelColor rgba)
																		// 		 label font

	void drawNeedle(float);

	float degToRad(float);						// Degrees to radians			(degrees)
	void processConfigLine(string, int);

};


#endif


