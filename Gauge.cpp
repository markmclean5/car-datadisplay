/********************************/
/*			Gauge Class			*/
/********************************/
#include <iostream>
#include "VG/openvg.h"		//
#include "VG/vgu.h"			//
#include "fontinfo.h"		// OpenVG
#include "shapes.h"			//
#include <cmath>			// Math (float remainder operation)
#include "TouchableObject.h"
#include "Gauge.h"			// Gauge 
#include <stdio.h>
#include <fstream>
#include <algorithm>
#include <stdlib.h>

#include <locale.h>
#include <config4cpp/Configuration.h>
#include "parsingUtilities.h"
using namespace std;
using namespace config4cpp;

void Gauge::configure(string gaugeType) {
	setlocale(LC_ALL, "");
	char * scope = "BoostGauge";

	Configuration * cfg = Configuration::create();

	try {
		cfg->parse("testConfig");
		string gaugeName = "BoostGauge";

		// Gauge attributes
		numRanges = parseInt(cfg, gaugeName, "numRanges");
		EngUnits = new std::string[numRanges];
		parseColor(cfg, gaugeName, borderColor, "borderColor");
		parseColor(cfg, gaugeName, backgroundColor, "backgroundColor");
		parseColor(cfg, gaugeName, needleColor, "needleColor");

		if(numRanges==0) numRanges=1;
		startVal = new float[numRanges];
		stopVal = new float[numRanges];
		startAng = new float[numRanges];
		stopAng = new float[numRanges];;
		majorInt = new float[numRanges];
 		minorInt = new float[numRanges];
		majorTickColor = new float*[numRanges];
		minorTickColor = new float*[numRanges];
		labelStartVal = new float[numRanges];;
		labelStopVal = new float[numRanges];
		labelIncrement = new float[numRanges];
		labelDecPlaces = new int[numRanges];
		labelStartAng = new float[numRanges];;
		labelStopAng = new float[numRanges];
		labelColor = new float*[numRanges];
		labelFont = new Fontinfo[numRanges];

		// Per-range attributes
		string rangeScope = "range";
		int currentRange = 1;
		for(;currentRange<=numRanges;currentRange++)
		{
			majorTickColor[currentRange-1] = new float[4];
			minorTickColor[currentRange-1] = new float[4];
			labelColor[currentRange-1] = new float[4];
			string currentRangeScope = rangeScope + to_string(currentRange);
			int prefixSize = currentRangeScope.length();
			string scope2 = scope;
			EngUnits[currentRange-1] = parseString(cfg, scope2, currentRangeScope, ".engUnits");
			majorInt[currentRange-1] = parseFloat(cfg, scope2, currentRangeScope, ".majorTickInterval");
			minorInt[currentRange-1] = parseFloat(cfg, scope2, currentRangeScope, ".minorTickInterval");
			startVal[currentRange-1] = parseFloat(cfg, scope2, currentRangeScope, ".dataRangeStart");
			stopVal[currentRange-1] = parseFloat(cfg, scope2, currentRangeScope, ".dataRangeStop");			
			startAng[currentRange-1] = parseFloat(cfg, scope2, currentRangeScope, ".angleRangeStart");
			stopAng[currentRange-1] = parseFloat(cfg, scope2, currentRangeScope, ".angleRangeStop");
			labelStartVal[currentRange-1] = parseFloat(cfg, scope2, currentRangeScope, ".labelValueStart");
			labelStopVal[currentRange-1] = parseFloat(cfg, scope2, currentRangeScope, ".labelValueStop");
			labelStartAng[currentRange-1] = parseFloat(cfg, scope2, currentRangeScope, ".labelAngleStart");
			labelStopAng[currentRange-1] = parseFloat(cfg, scope2, currentRangeScope, ".labelAngleStop");
			labelIncrement[currentRange-1] = parseFloat(cfg, scope2, currentRangeScope, ".labelIncrement");
			labelDecPlaces[currentRange-1] = parseFloat(cfg, scope2, currentRangeScope, ".labelDecPlaces");
			parseColor(cfg, scope2, currentRangeScope, majorTickColor[currentRange-1], ".majorTickColor");
			parseColor(cfg, scope2, currentRangeScope, minorTickColor[currentRange-1], ".minorTickColor");
			parseColor(cfg, scope2, currentRangeScope, labelColor[currentRange-1], ".labelColor");
		}
	} catch(const ConfigurationException & ex) {
		cout << ex.c_str() << endl;
	}
	cfg->destroy();
}

// Gauge Draw Method
void Gauge::draw(void)
{
	// Gauge sizing
	float borderWidth = 0.03 * radius;
	gaugeRadius = radius - borderWidth/2;
	dynamicContentRadius = 0.85 * gaugeRadius;	// MARK FOR REMOVAL

	StrokeWidth(0);
	setfill(backgroundColor);
	Circle(centerX,centerY,gaugeRadius*2);	// Draw gauge fill circle

	// Draw Ticks
	int range = 0;
	for(;range<numRanges;range++)
	{
		float valRange = stopVal[range] - startVal[range];
		float angRange = stopAng[range] - startAng[range];
		float angPerVal = angRange/std::abs(valRange);
		float angMajorInt = majorInt[range] * angPerVal;
		float angMinorInt = minorInt[range] * angPerVal;
		float angRatio = angMajorInt / angMinorInt;
		float* currentMajorTickColor = majorTickColor[range];
		float* currentMinorTickColor = minorTickColor[range];
		drawTickSet(startAng[range], stopAng[range], angMajorInt, angRatio, currentMajorTickColor, true);		// Draw major ticks
		drawTickSet(startAng[range], stopAng[range], angMinorInt, angRatio, currentMinorTickColor, false);		// Draw minor ticks
	}

	// Draw labels
	range = 0;
	for(;range<numRanges;range++)
	{
		float* currentLabelColor = labelColor[range];
		drawLabelSet(labelStartVal[range], labelStopVal[range], labelIncrement[range], labelDecPlaces[range], labelStartAng[range], labelStopAng[range], currentLabelColor, labelFont[range]);
	}
	StrokeWidth(borderWidth);
	Fill(0,0,0,0);
	setstroke(borderColor);
	Circle(centerX,centerY,gaugeRadius*2);	// Draw gauge border (on top of ticks)

	// Save gauge image in buffer
	GaugeBuffer = vgCreateImage(VG_sABGR_8888, 800, 480, VG_IMAGE_QUALITY_BETTER);
	vgGetPixels(GaugeBuffer, centerX-radius, centerY-radius, centerX - radius, centerY - radius, 2*radius, 2*radius);
}

void Gauge::update(float value, std::string units)
{
	updateVisuals();
	bool unitsFound = false;
	int range = 0;
	int dataRange = 0;
	while(!unitsFound)
	{
		if(range==numRanges)
		{	
			cout << "Error: Input units do not match any gauge range units." << endl;
			break;
		}
		if (EngUnits[range].compare(units) == 0)
		{
			unitsFound = true;
			dataRange = range;
		}
		range++;
	}

	if(unitsFound)
	{
		vgSeti(VG_IMAGE_MODE, VG_DRAW_IMAGE_MULTIPLY);
		float alphaScalar = (100. - getDesiredFadePercentage()) / 100.;
		Fill(255,255,255,alphaScalar);		// Alpha applied to vgDrawImage due to VG_DRAW_IMAGE_MULTIPLY
		vgDrawImage(GaugeBuffer);
		float needleAngle = -value * (stopAng[dataRange]-startAng[dataRange])/abs(stopVal[dataRange]-startVal[dataRange]);	
		drawNeedle(needleAngle);
	}	
}

void Gauge::drawNeedle(float angle)
{
	float alphaScalar = (100. - getDesiredFadePercentage()) / 100.;
	// Center Glow
	StrokeWidth(0);
	int glowRadius = gaugeRadius*0.1875;
	VGfloat glowStops[] = {	0.000,	needleColor[0],	needleColor[1],	needleColor[2],	alphaScalar,
							1.000,	0,				0,				0,				alphaScalar};
	FillRadialGradient(centerX, centerY, centerX, centerY, glowRadius+5, glowStops, 2);
	Circle(centerX, centerY, glowRadius*2);
	//Gauge Center
	float centerRadius = gaugeRadius*0.125;
	float scaling = gaugeRadius*0.041;
	StrokeWidth(0);
	Fill(0,0,0,alphaScalar);
	Circle(centerX, centerY, centerRadius);
	int focalX = centerX - 15;
	int focalY = centerY + 0;
	VGfloat stops[] = {	0.000,	5,	5,	5,	(float).05*alphaScalar,
						1.000,	0,	0,	0,	alphaScalar};
	FillRadialGradient(centerX, centerY, focalX, focalY, centerRadius*scaling, stops,2);
	Circle(centerX, centerY, centerRadius*2);
	Fill(0,0,0,.7*alphaScalar);
	Circle(centerX, centerY, centerRadius);
	Fill(0,0,0,alphaScalar);
	Circle(centerX, centerY, centerRadius*2*.75);
	// Gauge Needle
	centerRadius = gaugeRadius*0.05;
	float needleLength = 0.97*dynamicContentRadius;
	float xVertices[5];
	float yVertices[5];
	xVertices[0] = centerX - centerRadius*cos(degToRad(angle+90));
	yVertices[0] = centerY + centerRadius*sin(degToRad(angle+90));
	xVertices[1] = centerX - 0.92*needleLength*cos(degToRad(angle)) - (0.5*centerRadius)*cos(degToRad(angle+90));
	yVertices[1] = centerY + 0.92*needleLength*sin(degToRad(angle)) + (0.5*centerRadius)*sin(degToRad(angle+90));
	xVertices[2] = centerX - needleLength*cos(degToRad(angle));
	yVertices[2] = centerY + needleLength*sin(degToRad(angle));
	xVertices[3] = centerX - 0.92*needleLength*cos(degToRad(angle)) - (0.5*centerRadius)*cos(degToRad(angle-90));
	yVertices[3] = centerY + 0.92*needleLength*sin(degToRad(angle)) + (0.5*centerRadius)*sin(degToRad(angle-90));
	xVertices[4] = centerX - centerRadius*cos(degToRad(angle-90));
	yVertices[4] = centerY + centerRadius*sin(degToRad(angle-90));
	VGfloat needleStops[] = {	0.000,	needleColor[0], needleColor[1], needleColor[2], (float)0.1*alphaScalar,
								0.250,	needleColor[0], needleColor[1], needleColor[2], (float)0.8*alphaScalar,
								1.000,	needleColor[0], needleColor[1], needleColor[2], (float)0.8*alphaScalar};
	FillRadialGradient(centerX, centerY, centerX, centerY, needleLength, needleStops, 3);
	Polygon(xVertices, yVertices, 5);
	// Gauge Needle Cap
	Stroke(0,0,0,alphaScalar);
	StrokeWidth(2*centerRadius);
	float needleCapLength = centerRadius*3.33;
	float  needleCapX = centerX - needleCapLength*cos(degToRad(angle+180));
	float  needleCapY = centerY + needleCapLength*sin(degToRad(angle+180));
	Line(centerX, centerY, needleCapX, needleCapY);
}


void Gauge::drawLabelSet(float startLabel, float stopLabel, float labelIncrement, int decPlaces, float startAngle, float stopAngle, float* labelColor, Fontinfo font)
{
	float labelAngle = startAngle;
	if(labelIncrement == 0) labelIncrement = 1;
	int numLabels = (stopLabel - startLabel) / labelIncrement;
	int fontSize = 0.15*gaugeRadius;
	float labelRadius = 0.74*gaugeRadius;
	float angIncrement = (stopAngle-startAngle) / numLabels;
	setfill(labelColor);
	char labelText[10];
	char decPlacesText[10];
	
	sprintf(decPlacesText, "%d", decPlaces);
	string formatSpecifier = "%.";
	formatSpecifier.append(decPlacesText);
	formatSpecifier.append("f");
	char* formatSpecifierChar = (char*)formatSpecifier.c_str();

	float labelValue = startLabel;
	int labelCount = 0;
	for(;labelCount<=numLabels;labelCount++)
	{
		sprintf(labelText, formatSpecifierChar, labelValue);
		labelAngle = startAngle + labelCount*angIncrement;
		int labelX = centerX - labelRadius*cos(degToRad(labelAngle))-.195*fontSize;
		int labelY = centerY + labelRadius*sin(degToRad(labelAngle)) - fontSize/2;
		TextMid(labelX, labelY, labelText, font, fontSize);
		labelValue+=labelIncrement;
	}
}

void Gauge::drawTickSet(float startAng, float stopAng, float angInt, float angRatio, float* tickColor, bool isMajor)
{

	float tickWidth, tickLength, tickGlowWidth;
	if(isMajor)
	{
		tickLength = gaugeRadius-dynamicContentRadius;
		tickWidth = 0.2 * tickLength;
		tickGlowWidth = 0.3 * tickLength; 
	}
	else
	{
		tickLength = 0.65*(gaugeRadius-dynamicContentRadius);
		tickWidth = 0.2 * tickLength;
		tickGlowWidth = 0.3 * tickLength;
	}

	float tickGlowStops[] = {
		0.00,	tickColor[0],			tickColor[1],			tickColor[2],			tickColor[3]*(float)0.8,
		1.00,	backgroundColor[0],		backgroundColor[1],		backgroundColor[2],		backgroundColor[3]*(float)0.0
	};

	float angTotal = startAng;

	// Draw the Major / Minor Ticks
	Translate(centerX,centerY);
	Rotate(startAng);
	while(std::abs(angTotal-startAng)<=std::abs(stopAng-startAng))
	{
		if((remainder(angTotal, angInt*angRatio) == 0) && !isMajor)
		{
			Rotate(angInt);
			angTotal += angInt;
		}
		else
		{
			StrokeWidth(0);
			FillLinearGradient(-gaugeRadius,0.5*tickWidth, -gaugeRadius, 0.5*tickWidth+tickGlowWidth, tickGlowStops, 2);
			Rect(-gaugeRadius, 0.5*tickWidth, tickLength, 0.9*tickGlowWidth);
			FillLinearGradient(-gaugeRadius,-0.5*tickWidth, -gaugeRadius, -0.5*tickWidth-tickGlowWidth, tickGlowStops, 2);
			Rect(-gaugeRadius, -0.5*tickWidth-0.9*tickGlowWidth, tickLength, 0.9*tickGlowWidth);
			StrokeWidth(1.1*tickWidth);
			setstroke(tickColor);
			Line(-gaugeRadius, 0, -gaugeRadius+tickLength, 0);
			Rotate(angInt);
			angTotal += angInt;
		}
	}
	Rotate(-angTotal);
	Translate(-centerX,-centerY);
}

Gauge::Gauge(int x, int y, int rad)		// Constructor
{
	// Configuring base class (TouchableObject)
	setCircular();
	setCircleCenter(x, y);
	setCircleRadius(rad);

	centerX = x;
	centerY = y;
	radius = rad;
}

void Gauge::setNumRanges(int ranges)
{
	numRanges = ranges;

} 

void Gauge::setEngUnits(string units, int range)
{
	EngUnits[range-1] = units;
}

void Gauge::setBorderColor(float color[4])
{
	borderColor[0] = color[0];
	borderColor[1] = color[1];
	borderColor[2] = color[2];
	borderColor[3] = color[3];
}

void Gauge::setBackgroundColor(float color[4])
{
	backgroundColor[0] = color[0];
	backgroundColor[1] = color[1];
	backgroundColor[2] = color[2];
	backgroundColor[3] = color[3];
}

void Gauge::setNeedleColor(float color[4])
{
	needleColor[0] = color[0];
	needleColor[1] = color[1];
	needleColor[2] = color[2];
	needleColor[3] = color[3];
}
void Gauge::setDataRange(float start, float stop, int range)
{
	startVal[range-1]	= start;
	stopVal[range-1]	= stop;
}

void Gauge::setDataAngleRange(float start, float stop, int range)
{
	startAng[range-1]	= start;
	stopAng[range-1]	= stop;
}

void Gauge::setMajorInterval(float interval, int range)
{
	majorInt[range-1] = interval;
}

void Gauge::setMinorInterval(float interval, int range)
{
	minorInt[range-1] = interval;
}

void Gauge::setLabelRange(float start, float stop, int range)
{
	labelStartVal[range-1]	= start;
	labelStopVal[range-1]	= stop;
}

void Gauge::setLabelAngleRange(float start, float stop, int range)
{
	labelStartAng[range-1]	= start;
	labelStopAng[range-1]	= stop;
}

void Gauge::setLabelIncrement(float increment, int range)
{
	labelIncrement[range-1] = increment;
}

void Gauge::setLabelDecPlaces(int places, int range)
{
	labelDecPlaces[range-1] = places;
}

void Gauge::setLabelFont(Fontinfo font, int range)
{
	labelFont[range-1] = font;
}

float Gauge::degToRad(float degrees)
{
	float radians = (3.14159*degrees)/180.;
	return radians;
}
