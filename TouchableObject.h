#ifndef TOUCHABLEOBJECT_H
#define TOUCHABLEOBJECT_H 

#include <linux/input.h>
#include "touchscreen.h"

/**************************************************************************************************************
 * TouchableObject Class
 * Parent class for all display objects to handle touchscreen processing.
 *
 * Accepts:	Touch structure
 *
 * Determines if and how (TBD) a display object has been touched by keeping track of size, shape, and position
 **************************************************************************************************************/

class TouchableObject
{
private:
	bool isRectangular;	// Rectangle or circle? You decide.
	/* Circular properties */
	int cX;				// Circle center x coordinate
	int cY;				// Circle center y coordinate
	int cRad;			// Circle radius
	/* Rectangular properties */
	int rW;				// Rectangle width
	int rH;				// Rectangle height
	int rblX;			// Rectangle bottom-left corner x coordinate
	int rblY;			// Rectangle bottom-left corner y coordinate

	/* Touch processing & visibility */
	bool touchEnabled;
	bool visible;
	bool lpVisible;

	bool touched;


protected:
	/* Methods called by derived classes */
	bool getVisibility(void);				// Called by derived classes to determine visibility
	bool getLPVisibility(void);				// Called by derived classes to determine previous visiblity

	void setCircular(void);				// Called by derived class to set touch area as circular
	void setCircleCenter(int, int);		// Called by derived class to set circular touch area center
	void setCircleRadius(int);			// Called by derived class to set circular touch area radius

	void setRectangular(void);			// Called by derived class to set touch area as rectangular
	void setRectWidthHeight(int, int);	// Called by derived class to set rectangular touch area size
	void setRectBottomLeft(int, int);	// Called by derived class to set rectangular touch area bottom left cornet

	/* Constructor */
	TouchableObject(void);				// Sets up ClickableObject, sets properties to safe state

public:
	/* Control methods */
	void touchEnable(void);				// Enables touch functionality
	void touchDisable(void);			// Disables touch functionality
	void setVisible(void);				// Sets display object visible
	void setInvisible(void);			// Sets display object invisible

	/* Update method */
	void updateTouch(touch_t);

	/* All that for this.. */
	bool isTouched(void);
};

#endif