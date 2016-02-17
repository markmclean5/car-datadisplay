/********************************/
/*	PID Class					*/
/********************************/
#include <sys/types.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <cmath>			// Math (float remainder operation)
#include <math.h>
#include "PID.h"
#include <bcm2835.h>
#include <algorithm>    // std::find
#include <string>


#include <locale.h>
#include <config4cpp/Configuration.h>
#include "parsingUtilities.h"

using namespace std;
using namespace config4cpp;


PID::PID(string PIDid) {
	cout << "PID created: " << PIDid << endl; 

	id = PIDid;
	debug = false;

	currentRange = 1;
	lastRange = 1;

	configure(PIDid);

	currentTime = 0;
	lastTime = 0;
	updateRate = 0;
	desiredUpdateRate = 0;
	lastUpdateTime = 0;
}

/* PID configure method */
void PID::configure(string ident) {
	setlocale(LC_ALL, "");
	Configuration * cfg = Configuration::create();
	try {
		cfg->parse("/home/pi/openvg/client/PIDConf");
		string PIDName = ident;
		fullName = parseString(cfg, PIDName, "fullName");
		shortName = parseString(cfg, PIDName, "shortName");
		command = parseString(cfg, PIDName, "command");

		numRanges = parseInt(cfg, PIDName, "numRanges");

		supportedMinVal = parseFloat(cfg, PIDName, "supportedMinVal");
		supportedMaxVal = parseFloat(cfg, PIDName, "supportedMaxVal");
		numDataBytes = parseInt(cfg, PIDName, "numDataBytes");

		byteGain[0] = parseFloat(cfg, PIDName, "AGain");
		byteOffset[0] = parseFloat(cfg, PIDName, "AOffset");
		byteGain[1] = parseFloat(cfg, PIDName, "BGain");
		byteOffset[1] = parseFloat(cfg, PIDName, "BOffset");
		byteGain[2] = parseFloat(cfg, PIDName, "CGain");
		byteOffset[2] = parseFloat(cfg, PIDName, "COffset");
		byteGain[3] = parseFloat(cfg, PIDName, "DGain");
		byteOffset[3] = parseFloat(cfg, PIDName, "DOffset");

		TotalGain = parseFloat(cfg, PIDName, "TotalGain");
		TotalOffset = parseFloat(cfg, PIDName, "TotalOffset");

		cout << "Num Ranges set to: " << numRanges << endl;
		
		rangeScaling = new float[numRanges];
		rangeStart = new float[numRanges];
		rangeStop = new float[numRanges];
		EngUnits = new string[numRanges];
		weightedMALag = new int[numRanges];
		simpleMALag = new int[numRanges];
		weightedMACoeffs = new float*[numRanges];
		weightedMAData = new float*[numRanges];
		simpleMAData = new float*[numRanges];

		
		string rangeScope = "range";
		currentRange = 1;
		for(;currentRange<=numRanges;currentRange++) {
			string currentRangeScope = rangeScope + to_string(currentRange);
			rangeScaling[currentRange-1] = parseFloat(cfg, PIDName, currentRangeScope, ".scaling");
			rangeStart[currentRange-1] = parseFloat(cfg, PIDName, currentRangeScope, ".rangeStart");
			rangeStop[currentRange-1] = parseFloat(cfg, PIDName, currentRangeScope, ".rangeStop");
			EngUnits[currentRange-1] = parseString(cfg, PIDName, currentRangeScope, ".engUnits");
			simpleMALag[currentRange-1] = parseInt(cfg, PIDName, currentRangeScope, ".simpleMALag");
			simpleMAData[currentRange-1] = new float[simpleMALag[currentRange-1]];

			float* currentMAData = simpleMAData[currentRange-1];
			for(int i = 0; i<simpleMALag[currentRange-1]; i++)
				currentMAData[i] = 0;

			weightedMALag[currentRange-1] = parseInt(cfg, PIDName, currentRangeScope, ".weightedMALag");
			weightedMAData[currentRange-1] = new float[weightedMALag[currentRange-1]];

			currentMAData = weightedMAData[currentRange-1];
			for(int i = 0; i<weightedMALag[currentRange-1]; i++)
				currentMAData[i] = 0;

			weightedMACoeffs[currentRange-1] = new float[weightedMALag[currentRange-1]];
			parseFloatArray(cfg, PIDName, currentRangeScope, weightedMALag[currentRange-1], weightedMACoeffs[currentRange-1], ".weightedMACoeffs");

		}
		currentRange = 1;
		
	}catch(const ConfigurationException & ex) {
		cout << ex.c_str() << endl;
	}
	cfg->destroy();

	cout << "Current range at end of configure: " << currentRange << endl;
}


string PID::getCommand(void) {
	return command;
}


// Getters: DataStream outputs
string PID::getEngUnits(void) {
	return EngUnits[currentRange-1];
}

float PID::getRawDatum(void) {
	cout << "get raw datum called for "<< getCommand() <<"  - range: " << currentRange << endl;
	float* currentSimpleMAData = simpleMAData[currentRange-1];
	return currentSimpleMAData[simpleMALag[currentRange-1]-1];			// returns most recent value
}
float PID::getWeightedMADatum(void)
{
	float* currentWeightedMAData = weightedMAData[currentRange-1];
	float* currentWeightedMACoeffs = weightedMACoeffs[currentRange-1];
	float datum = 0;
	float coeffSum = 0;
	int idx = 0;
	for(;idx<weightedMALag[currentRange-1];idx++)
	{
		datum += (currentWeightedMAData[idx])* (currentWeightedMACoeffs[idx]);
		coeffSum += currentWeightedMACoeffs[idx];
	}
	if (coeffSum == 0) coeffSum = 1;
	datum = datum/coeffSum;
	return datum;
}
float PID::getSimpleMADatum(void)
{
	float* currentSimpleMAData = simpleMAData[currentRange-1];
	float datum = 0;
	int idx = 0;
	for(;idx<simpleMALag[currentRange-1];idx++) datum += (currentSimpleMAData[idx] *  (float)(1 / simpleMALag[currentRange-1]));
	return datum;
}
float PID::getReadoutDatum(void)
{
	return 0;
}
float PID::getRawUpdateRate(void)
{
	return updateRate;
}
float PID::getReadoutUpdateRate(void)
{
	uint64_t currentTime = bcm2835_st_read();
	if(desiredUpdateRate == 0)	desiredUpdateRate = 5;
	uint64_t nextTime = lastUpdateTime + (1000000/desiredUpdateRate);
	if(currentTime>=nextTime) {
		readoutUpdateRate = getRawUpdateRate();
		lastUpdateTime = currentTime;
	}

	return readoutUpdateRate;
}

void PID::update (string serialData, uint64_t updateTime) {
	cout << "PID update called for: " << getCommand() << endl;
	currentTime = updateTime;
	uint64_t timeDelta = currentTime - lastTime;
	lastTime = currentTime;
	updateRate = 1000000./timeDelta;
	string responseID = id;
	responseID[0] = '4';
	if(!serialData.empty()){
		// Remove spaces from serial data string (this might not work)
		std::string::iterator end_pos = std::remove(serialData.begin(), serialData.end(), ' ');
		serialData.erase(end_pos, serialData.end());
		size_t found = serialData.find(responseID);
		if(found != std::string::npos) {
			float value = 0;
			string dataByteString = serialData.substr(found+4, numDataBytes*2);
			cout << "Data Byte String - " << dataByteString << endl;
			cout << "num data bytes: " << numDataBytes << endl;
			for(int i=0;i<numDataBytes;i++) {
				cout << "byte gain " << byteGain[i] << endl;
				cout << "byte offset " << byteOffset[i] << endl;
				cout << " substring " <<  dataByteString.substr(2*i, 2).c_str() << endl;
				cout << "stroutl " << strtoul(dataByteString.substr(2*i, 2).c_str(), NULL, 16) << endl;
				value += (byteGain[i]*strtoul(dataByteString.substr(2*i, 2).c_str(), NULL, 16) + byteOffset[i]);
				cout << "value before total " << value << endl;
			}
			cout << "TotalGain " << TotalGain << endl;
			cout << "TotalOffset " << TotalOffset << endl;
			value = (value*TotalGain) + TotalOffset;
			cout << " Value calculated to be: " << value <<endl;
			// Now find out which range the  value belongs to
			int range = 0;
			bool rangeFound = false;
			while(!rangeFound) {
				if(range==numRanges) break;
				if(debug) {
					cout << "Checking if data fits within range # " << range+1 << endl;
					cout << "Range start: " << rangeStart[range] << endl;
					cout << "Range stop: " << rangeStop[range] << endl;
				}	
				if(((value>=rangeStart[range]) && (value<rangeStop[range])) ||
					((value>=rangeStop[range]) && (value<rangeStart[range]))) {
					if(debug) cout << "Data found in range # " << range+1 << endl;
					lastRange = currentRange;
					currentRange = range+1;
					rangeFound = true;
				}
				range++;
			}
			if(rangeFound==true) {
				value = value * rangeScaling[currentRange-1];	// Apply scaling of current range
				if(debug) cout << "Value scaled to range: " << value <<endl;
				if(debug) cout << "Renge units: " << EngUnits[currentRange-1] << endl;
				float* currentSimpleMAData = simpleMAData[currentRange-1];
				float* currentWeightedMAData = weightedMAData[currentRange-1];
				// If the range has changed this time around, reset MA data to current value
				int idx;
				if(currentRange!=lastRange) {
					idx = 0;
					for(;idx<simpleMALag[currentRange-1];idx++) currentSimpleMAData[idx] = value;
					idx = 0;
					for(;idx<weightedMALag[currentRange-1];idx++) currentWeightedMAData[idx] = value;
				}
				// Shift all simple MA data values down one, put current value in last position
				idx = 1;							
				for(;idx<simpleMALag[currentRange-1];idx++)
					currentSimpleMAData[idx-1] = currentSimpleMAData[idx];	// Shift data, overwrite oldest
				currentSimpleMAData[simpleMALag[currentRange-1]-1] = value;	// Add latest value to array
				// Shift all weighted MA data values down one, put current value in last position
				idx = 1;
				for(;idx<weightedMALag[currentRange-1];idx++)	
					currentWeightedMAData[idx-1] = currentWeightedMAData[idx];	// Shift data, overwrite oldest
				currentWeightedMAData[weightedMALag[currentRange-1]-1] = value;	// Add latest value to array
			}
			else {
				cout << "Error: Data not inside any range. " << endl;
				currentRange=1;
			}
		}


	}

	else {
		currentRange = 1;
		cout << "PID updated without data" << endl;
	}

	cout << "At end of PID update: current range: " << currentRange << endl;
}


