/********************************/
/*		Readout Class			*/
/********************************/
using namespace std;

class Readout
{
private:
	// Timing related properties
	int desiredRefreshRate;
	uint64_t lastUpdateTime;

	// Graphics related properties
	float* valueColor;
	float* labelColor;
	float* backgroundColor;
	float* borderColor;
	int borderWidth;
	int fontSize;
	string label;
	char* formatSpecifier;
	int numLines;
	char labelAlign;
	char valueAlign;
	int decPlaces;

	// Size & location related properties
	int readoutWidth, readoutHeight;		// Size of entire readout, including border stroke
	int rectWidth, rectHeight;				// Size of rectangle to be drawn
	int centerX, centerY;					// Location of readout center
	int bottomLeftX, bottomLeftY;			// Location of rectangle bottom left corner

public:
	Readout(int, int, int, int, int);		// Readout constructor: center X, center Y, width, height
	void setBackgroundColor(float*);
	void setBorder(float*, int);			// Set border color, border width
	void setDecPlaces(int);					// Set number of digits after decimal
	void setRefreshRate(int);				// Set desired refresh frequency (Hz) 
	void alignValue(char);					// 'L', 'R', 'C' for left, right, center value alignment
	void alignLabel(char);					// 'L', 'R', 'C' for left, right, center label alignment
	void setLabel(string);					// Set readout label
	void draw(void);						// Draw readout
	void update(float);						// Draw readout with new data
};