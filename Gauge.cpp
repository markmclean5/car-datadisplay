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

using namespace std;
// Prototype for processConfigLine method
void Gauge::configure(string gaugeType) {
	ifstream configFile;
	configFile.open("configGauges");
	if(!configFile){
		cout << "Unable to open config file" << endl;
		exit(1);
	}
	string currentLine;
	string searchString = "name";
	bool typeFound = false;
	size_t pos;
	int currentRange = 0;
	while(configFile.good()) {
    	getline(configFile, currentLine);
    	if(!typeFound) {
    		pos=currentLine.find(searchString);
    		if(pos!=string::npos) {	// string::npos is returned if string is not found
    			pos=currentLine.find(gaugeType);
    			if(pos!=string::npos){
    				cout << "Gauge Type " << gaugeType << " found in this name line:" << endl;
    				cout << currentLine << endl;
    				typeFound = true;
    			}
    			else{
    				cout << "Gauge Type " << gaugeType << " not found this the name line:" <<endl;
    				cout << currentLine << endl;
    				cout << "Continuing search.." << endl;
    			}
			}
		}
		else {
			if(!currentLine.empty()){
				currentLine.erase(remove(currentLine.begin(), currentLine.end(), ' '), currentLine.end());
				//cout << "Without spaces: " << currentLine << endl;
				currentLine.erase(remove(currentLine.begin(), currentLine.end(), '	'), currentLine.end());
				//cout << "Without tabs and spaces: " << currentLine << endl;
			}


			if(!currentLine.empty()) {
				pos = currentLine.find(":END:");
				if (pos!=string::npos)
					break;
				pos = currentLine.find("RANGE=");
				string rangeString;
				if(pos!=string::npos){
					size_t rangeEnd = currentLine.find(";");
					if(rangeEnd!=string::npos){
						rangeString.insert(0,currentLine, 6, rangeEnd-6);
						cout << "Range String" << rangeString << endl;
						currentRange = stoi(rangeString);
						cout << "Range Set to " << currentRange << endl;
					}
				}
				else processConfigLine(currentLine, currentRange);
			} 
		}
	}
}

void Gauge::processConfigLine(string inputLine, int range) {


		size_t firstEquals = inputLine.find("=");
		size_t firstSemicolon = inputLine.find(";");
		size_t attributeEndChar = firstEquals - 1;
		string configAttribute;
		configAttribute.insert(0, inputLine, 0, attributeEndChar+1);
		cout << "Config attribute: " << configAttribute << "*" << endl;
		string attributeContents;
		attributeContents.insert(0, inputLine, firstEquals+1, firstSemicolon - (firstEquals+1));
		cout << "	Contents: " << attributeContents << "*" << endl;
		
		// String handling
		if(attributeContents[0] == '"') {
			cout << "This is a string" << endl;
			string contents;
			contents.insert(0, attributeContents, 1, attributeContents.length()-2);
			cout << "String contents: " << contents << endl;
			
			
			if(configAttribute.compare("engUnits")==0){
				setEngUnits(contents, range);
				cout << "Set engineering units for range " << range << endl;
			}
			
		}
		// Array handling
		else if (attributeContents[0] == '{') {
			int* commaLocations;
			string* valueStrings;
			float* values;
			int itemCount = 1;
			
			size_t searchResult;
			int searchStart = 0;
			while(searchResult!=string::npos)
			{
				searchResult = attributeContents.find(",", searchStart);
				if(searchResult!=string::npos) {
					searchStart = searchResult+1;
					itemCount++;
				}

			}
			cout << "This is an array with "<< itemCount <<" elements"<< endl;

			commaLocations = new int[itemCount-1];
			valueStrings = new string[itemCount];
			values = new float[itemCount];

			searchStart = 0;
			int idx = 0;
			size_t commaSearchResult;
			while(commaSearchResult!=string::npos)
			{
				commaSearchResult = attributeContents.find(",", searchStart);
				if(commaSearchResult!=string::npos) {
					searchStart = commaSearchResult+1;
					commaLocations[idx] = commaSearchResult;
					idx++;
					cout << "Comma Search Loop Cycle End" <<endl;
				}
			}
			idx = 0;
			int start;
			int end;
			for(;idx<itemCount;idx++){
				if(idx == 0) {
					start = 1;
					end = commaLocations[idx];
					cout << "First valueString start and end set" <<endl;
				}
				else if(idx == itemCount-1) {
					start = commaLocations[idx-1]+1;
					end = attributeContents.size()-1;
					cout << "Last valueString start and end set" <<endl;
				}
				else {
					start = commaLocations[idx-1]+1;
					end =  commaLocations[idx];
					cout << "Intermediate valueString start and end set" <<endl;
				}
				cout << "Length = " << end-start << endl;
				valueStrings[idx].insert(0, attributeContents, start, end-start);
				cout << "Value string #" << idx+1 << "= " << valueStrings[idx] << endl;
				values[idx] = stof(valueStrings[idx]);
				cout << "Value: " << values[idx] << endl;
			}

			// Call appropriate setters which take in arrays


		}

		else cout << "This is a float" << endl;



		configAttribute = "";
		attributeContents = "";
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
		float majorTickColor[] =	{majorTickColorRed[range],
									 majorTickColorGreen[range],
									 majorTickColorBlue[range],
									 majorTickColorAlpha[range]};
		float minorTickColor[] =	{minorTickColorRed[range],
									 minorTickColorGreen[range],
									 minorTickColorBlue[range],
									 minorTickColorAlpha[range]};
		drawTickSet(startAng[range], stopAng[range], angMajorInt, angRatio, majorTickColor, true);		// Draw major ticks
		drawTickSet(startAng[range], stopAng[range], angMinorInt, angRatio, minorTickColor, false);		// Draw minor ticks
	}

	// Draw labels
	range = 0;
	for(;range<numRanges;range++)
	{
		float labelColor[] =	{labelColorRed[range],
								 labelColorGreen[range],
								 labelColorBlue[range],
								 labelColorAlpha[range]};
		drawLabelSet(labelStartVal[range], labelStopVal[range], labelIncrement[range], labelDecPlaces[range], labelStartAng[range], labelStopAng[range], labelColor, labelFont[range]);
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
	if(numRanges==0) numRanges=1;
	startVal = new float[numRanges];
	stopVal = new float[numRanges];
	startAng = new float[numRanges];
	stopAng = new float[numRanges];;
	majorInt = new float[numRanges];
 	minorInt = new float[numRanges];
	majorTickColorRed = new float[numRanges];
	majorTickColorGreen = new float[numRanges];
	majorTickColorBlue = new float[numRanges];
	majorTickColorAlpha = new float[numRanges];
	minorTickColorRed = new float[numRanges];
	minorTickColorGreen = new float[numRanges];
	minorTickColorBlue = new float[numRanges];
	minorTickColorAlpha = new float[numRanges];

	labelStartVal = new float[numRanges];;
	labelStopVal = new float[numRanges];
	labelIncrement = new float[numRanges];
	labelDecPlaces = new int[numRanges];
	labelStartAng = new float[numRanges];;
	labelStopAng = new float[numRanges];
	labelColorRed = new float[numRanges];
	labelColorGreen = new float[numRanges];
	labelColorBlue = new float[numRanges];
	labelColorAlpha = new float[numRanges];
	labelFont = new Fontinfo[numRanges];
	EngUnits = new std::string[numRanges];
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

void Gauge::setMajorTickColor(float color[4], int range)
{
	majorTickColorRed[range-1]		= color[0];
	majorTickColorGreen[range-1]	= color[1];
	majorTickColorBlue[range-1]		= color[2];
	majorTickColorAlpha[range-1]	= color[3];
}

void Gauge::setMinorTickColor(float color[4], int range)
{
	minorTickColorRed[range-1]		= color[0];
	minorTickColorGreen[range-1]	= color[1];
	minorTickColorBlue[range-1]		= color[2];
	minorTickColorAlpha[range-1]	= color[3];
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

void Gauge::setLabelColor(float color[4], int range)
{
	labelColorRed[range-1]			= color[0];
	labelColorGreen[range-1]		= color[1];
	labelColorBlue[range-1]			= color[2];
	labelColorAlpha[range-1]		= color[3];
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
