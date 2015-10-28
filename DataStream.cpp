/********************************/
/*	DataStream Class			*/
/********************************/
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <cmath>			// Math (float remainder operation)
#include <math.h>
#include "DataStream.h"	
#include <bcm2835.h>
#include <algorithm>    // std::find
#include <string>

using namespace std;

DataStream::DataStream(int ranges)
{
	debug = false;
	numRanges = ranges;
	cout << "Num Ranges set to: " << numRanges << endl;
	rangeStart = new float[numRanges];
	rangeStop = new float[numRanges];
	rangeScaling = new float[numRanges];
	EngUnits = new string[numRanges];
	weightedMALag = new int[numRanges];
	simpleMALag = new int[numRanges];
	weightedMACoeffs = new float*[numRanges];
	weightedMAData = new float*[numRanges];
	simpleMAData = new float*[numRanges];

	currentRange = 1;
	lastRange = 1;

	currentTime = 0;
	lastTime = 0;
	updateRate = 0;
}

void DataStream::setDebugMode(bool debugMode)
{
	debug = debugMode;
}

void DataStream::setRangeLimits(float min, float max, int range)
{
	rangeStart[range-1] = min;
	rangeStop[range-1] = max;
	if(debug)
	{
		cout << "Range Lower Limit set to: " << rangeStart[range-1] << endl;
		cout << "Range Upper Limit set to: " << rangeStop[range-1] << endl;
	}
}

void DataStream::setRangeScaling(float scaling, int range)
{
	rangeScaling[range-1] = scaling;
	if(debug)
	{
	cout << "Range Scaling set to: " << rangeScaling[range-1] << endl;
	}
}

void DataStream::setEngUnits(string eu, int range)
{
	EngUnits[range-1] = eu;
	cout << "Engineering units set to: " << EngUnits[range-1] << endl;
}


void DataStream::setCharTags(char start, char stop)
{
	startCharTag = start;
	stopCharTag = stop;
}

void DataStream::setStreamScaling(float scaling)
{
	streamScaling = scaling;
}
void DataStream::setWeightedMALag(int lag, int range)
{
	if (lag == 0) lag = 1;
	weightedMALag[range-1] = lag;
	weightedMAData[range-1] = new float[lag];
	weightedMACoeffs[range-1] = new float[lag];
	int idx = 0;

	float* currentMAData = weightedMAData[range-1];
	float* currentMACoeffs = weightedMACoeffs[range-1];

	for(;idx<lag;idx++)
	{
		currentMAData[idx] = 0;
		currentMACoeffs[idx] = 0;
	}
	cout << "Weighted MA Lag set to: " << weightedMALag[range-1] << endl;
}
void DataStream::setWeightedMACoeffs(float* coeffs, int range)
{
	int idx = 0;

	float* currentMACoeffs=weightedMACoeffs[range-1];
	for(;idx<weightedMALag[range-1];idx++)
		currentMACoeffs[idx] = coeffs[idx];
	cout << "Weighted MA Coeffs set. " << endl;
}
void DataStream::setSimpleMALag(int lag, int range)
{
	if (lag == 0) lag = 1;
	simpleMALag[range-1] = lag;
	simpleMAData[range-1] = new float[lag];
	int idx = 0;
	float* currentMAData = simpleMAData[range-1];

	for(;idx<lag;idx++) currentMAData[idx] = 0;
	cout << "Simple MA Lag set. " << endl;
}
void DataStream::setReadoutFreq(int freq)
{
	readoutFreq = freq;
}


// Getters: DataStream outputs
string DataStream::getEngUnits(void)
{
	return EngUnits[currentRange-1];
}

float DataStream::getRawDatum(void)
{
	float* currentSimpleMAData = simpleMAData[currentRange-1];
	return currentSimpleMAData[simpleMALag[currentRange-1]-1];			// returns most recent value
}
float DataStream::getWeightedMADatum(void)
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
float DataStream::getSimpleMADatum(void)
{
	float* currentSimpleMAData = simpleMAData[currentRange-1];
	float datum = 0;
	int idx = 0;
	for(;idx<simpleMALag[currentRange-1];idx++) datum += (currentSimpleMAData[idx] *  (1 / simpleMALag[currentRange-1]));
	return datum;
}
float DataStream::getReadoutDatum(void)
{
	return 0;
}
float DataStream::getRawUpdateRate(void)
{
	return updateRate;
}
float DataStream::getReadoutUpdateRate(void)
{
	return 0;
}

void DataStream::update (char* serialData, uint64_t updateTime)
{
		currentTime = updateTime;
		uint64_t timeDelta = currentTime - lastTime;
		lastTime = currentTime;
		updateRate = 1000000./timeDelta;


		if(*serialData != 0)		//check if null pointer
		{
			int serialDataSize = 0;
			if(debug)
			{
				printf("This is the address of the buffer: %p\n",serialData);
				printf("This is the content of the buffer:");
			}
			for( ; serialData[serialDataSize]!=0; serialDataSize++)
			{
				if(debug)
				{
					printf("%c", serialData[serialDataSize]);
					printf("\n");
				}
			}
			
			char * startCharPtr = std::find(serialData, serialData+serialDataSize, startCharTag);

			if (startCharPtr != serialData+serialDataSize)
			{
				if(debug) std::cout << "Start character tag found: " << *startCharPtr << '\n';
				char * stopCharPtr = std::find(serialData, serialData+serialDataSize, stopCharTag);
				if (stopCharPtr != serialData+serialDataSize)
				{
					if (debug) std::cout << "Stop character tag found: " << *stopCharPtr << '\n';
					char dataChars[stopCharPtr-startCharPtr];
					int idx = 0;
					for(;idx<(stopCharPtr-startCharPtr-1);idx++) dataChars[idx] = *(startCharPtr+1+idx);
					dataChars[stopCharPtr-startCharPtr-1] = '\0';
					if (streamScaling == 0) streamScaling = 1;
					float scaledValue = atof(dataChars) / streamScaling;
					if (debug) cout << "Scaled value: " << scaledValue << endl;

					// Now find out which range the scaled value belongs to
					int range = 0;
					bool rangeFound = false;
					while(!rangeFound)
					{
						if(range==numRanges) break;

						if(debug)
						{
							cout << "Checking if data fits within range # " << range+1 << endl;
							cout << "Range start: " << rangeStart[range] << endl;
							cout << "Range stop: " << rangeStop[range] << endl;
						}
						
						if(((scaledValue>=rangeStart[range]) && (scaledValue<rangeStop[range])) ||
							((scaledValue>=rangeStop[range]) && (scaledValue<rangeStart[range])))
						{
							if(debug) cout << "Data found in range # " << range+1 << endl;
							lastRange = currentRange;
							currentRange = range+1;
							rangeFound = true;
						}
						range++;
					}

					if(rangeFound==true)
					{
						scaledValue = scaledValue * rangeScaling[currentRange-1];	// Apply scaling of current range
						if(debug) cout << "Value scaled to range: " << scaledValue <<endl;
						if(debug) cout << "Renge units: " << EngUnits[currentRange-1] << endl;
						float* currentSimpleMAData = simpleMAData[currentRange-1];
						float* currentWeightedMAData = weightedMAData[currentRange-1];
						// If the range has changed this time around, reset MA data to current value
						if(currentRange!=lastRange)									
						{
							idx = 0;
							for(;idx<simpleMALag[currentRange-1];idx++) currentSimpleMAData[idx] = scaledValue;
							idx = 0;
							for(;idx<weightedMALag[currentRange-1];idx++) currentWeightedMAData[idx] = scaledValue;
						}
						// Shift all simple MA data values down one, put current value in last position
						idx = 1;							
						for(;idx<simpleMALag[currentRange-1];idx++)
							currentSimpleMAData[idx-1] = currentSimpleMAData[idx];	// Shift data, overwrite oldest
						currentSimpleMAData[simpleMALag[currentRange-1]-1] = scaledValue;	// Add latest value to array
						// Shift all weighted MA data values down one, put current value in last position
						idx = 1;
						for(;idx<weightedMALag[currentRange-1];idx++)	
							currentWeightedMAData[idx-1] = currentWeightedMAData[idx];	// Shift data, overwrite oldest
						currentWeightedMAData[weightedMALag[currentRange-1]-1] = scaledValue;	// Add latest value to array	
					}
					else cout << "Error: Data not inside any range. " << endl;

				}
			}

		}
}