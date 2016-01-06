#ifndef DATASTREAM_H
#define DATASTREAM_H

/**************************************************************************************************************
 * DataStream Class
 * Responsible for processing input data received over serial and processing the data into
 * engineering units.
 *
 * Provides:	Last raw datum recieved
 *				Weighted moving average of data using provided coefficents and lag
 *				Simple moving average of data using provided lag
 *				Capability to set multiple ranges of data output in different eng. units
 *					Note: Ranges must not overlap. Example: For boost gauge processing,
 *					input data is recieved in PSI. Positive input data is output in "PSI",
 *					and negative pressure data is scaled and reported in "inHg".
 *
 **************************************************************************************************************/


class DataStream
{
private:	// Class Private properties
	
	// Class properties
	bool debug;
	int numRanges;
	int currentRange;
	int lastRange;

	// Input stream processing properties
	char startCharTag;
	char stopCharTag;
	float streamScaling;

	// Weighted moving average properties
	int* weightedMALag;
	float** weightedMACoeffs;
	float** weightedMAData;

	// Simple moving average properties
	int* simpleMALag;
	float** simpleMAData;

	// Data range related properties
	float* rangeStart;			// min value of every range
	float* rangeStop;			// max value of every range
	float* rangeScaling;		// scaling value of every range
	std::string* EngUnits;		// engineering units of every range
	int* dataCounts;			// Number of data counts in each data range between range switch

	// Timing related properties
	float readoutFreq;

	// Add more timing replated properties
	uint64_t lastTime;
	uint64_t currentTime;
	uint64_t lastUpdateTime;
	float updateRate;			// The rate at which DataStream.update() is being called 
	float readoutUpdateRate;
	int desiredUpdateRate; 

public:		// Class members
	// Setters (call before update!!!!)

	DataStream(int); 							// DataStream constructor, sets number of ranges (required for operation)
	void setDebugMode(bool);					

	void setCharTags(char, char);				// Set serial stream start and stop characters		(start char, stop char)
	void setStreamScaling(float);						// Set stream scaling (convert to eng. units)		(set)
	void setRangeLimits(float,float, int);		// Set data range limits 		(min, max, range #)
	void setRangeScaling(float, int);			// Set range scaling, range
	void setEngUnits(std::string, int);				// Set engineering unit string, range


	// Weighted Moving Average Setters
	void setWeightedMALag(int, int);			// Weighted MA lag, range
	void setWeightedMACoeffs(float*, int);		// Weighted MA coeffs, range

	// Simple Moving Average Setter
	void setSimpleMALag(int, int);				// Simple MA lag, range

	// Setters readout update frequency
	void setReadoutFreq(int);					

	
	// Getters for DataStream data
	float getRawDatum(void);
	float getWeightedMADatum(void);
	float getSimpleMADatum(void);
	float getReadoutDatum(void);
	std::string getEngUnits(void);

	std::string getLogFormatData();

	// Getters for DataStream timing info
	float getRawUpdateRate(void);
	float getReadoutUpdateRate(void);

	// Update function
	void update(char*, uint64_t);				// Update method									(serial stream, time)
	


private:	// Class private members

};
#endif