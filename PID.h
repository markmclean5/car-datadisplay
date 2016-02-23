#ifndef PID_H
#define PID_H

/**************************************************************************************************************
 * PID Class
 * Data class for processing OBD2 PID data received and processing the data into engineering units.
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


class PID {
private:	// Class Private properties

	std::string fullName;
	std::string shortName;
	std::string id;
	int numRanges;

	

	float supportedMinVal, supportedMaxVal;
	int numDataBytes;
	float byteGain[4];
	float byteOffset[4];
	float TotalGain, TotalOffset;


	// Range Properties
	float* rangeScaling;		// scaling value of every range
	float* rangeStart;			// min value of every range
	float* rangeStop;			// max value of every range
	std::string* EngUnits;		// engineering units of every range
	// Simple moving average properties
	int* simpleMALag;
	float** simpleMAData;
	// Weighted moving average properties
	int* weightedMALag;
	float** weightedMACoeffs;
	float** weightedMAData;

	// Class properties
	bool debug;
	int currentRange;
	int lastRange;

	// Timing related properties
	float readoutFreq;

	// Add more timing replated properties
	uint64_t lastTime;
	uint64_t currentTime;
	uint64_t lastUpdateTime;
	float updateRate;			// The rate at which PID.update() is being called 
	float readoutUpdateRate;
	int desiredUpdateRate;


	// Bitwise encoded PIDs
	std::string type;
	uint32_t bitValue;
	std::string bitNames[32];
	std::string bit0States[32];
	std::string bit1States[32];
	




public:		// Class members
	// Setters (call before update!!!!)

	PID(std::string); 							// PID constructor, accepts name of identifer to find PID configuration)
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


	// Getters for DataStream timing info
	float getRawUpdateRate(void);
	float getReadoutUpdateRate(void);

	// Update function
	void update(std::string, uint64_t);				// Update method									(serial stream, time)
	std::string getCommand(void);
	std::string command;


	// Bit-encoded functions
	bool getBitNameValue(std::string);
	std::string getBitNameState(std::string);
	bool getBitPositionValue(int);
	std::string getBitPositionState(int);
	std::string getBitPositionName(int);
	int getNumBits(void);


private:	// Class private members

	void configure(std::string);						// PID configure method

};
#endif