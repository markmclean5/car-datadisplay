#ifndef TOUCHABLEOBJECT_H
#define TOUCHABLEOBJECT_H 

#include <linux/input.h>
#include "touchscreen.h"

/**************************************************************************************************************
 * TouchableObject Class
 * Parent class for all display objects to process:
 *		- Touchscreen processing:	Allows object to return touch state and touch data
 *		- Display visibility: 		Allows object to be set visible and visible
 *		- Movement on screen:		Provides command set for moving object on display
 *
 * Derived class configures TouchableObject using protected methods
 * Derived class querys position for movement commands, TouchableObject class manages movement
 *
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
	int rX;				// Rectangle center x coordinate
	int rY;				// Rectangle center y coordinate

	/* Touch processing properties */
	bool touchEnabled;	// Enables touch processing
	bool touched;		// Current touch state

	/* Visibility properties */
	bool visible;		// Current visibility state
	bool lpVisible;		// Previous visibility state

	/* Movement Properties */
	bool movingTo;			// In process of moving object to a provided position
	bool movingOffRight;	// In process of moving object off of the screen to the right
	bool movingOnRight;		// In process of moving object onto the screen from the right
	bool movingOffLeft;		// In process of moving object off of the screen to the left
	bool movingOnLeft;		// In process of moving object onto the screen from the left
	bool movingOffBottom;	// In process of moving object off of the screen to the bottom
	bool movingOnBotttom;	// In process of moving object onto the screen from the bottom
	bool movingOffTop;		// In process of moving object off of the screen to the top
	bool movingOnTop;		// In process of moving object onto the screen from the top

	/* Previous position properties */
	// Set at the beginning of a moveOff, used as final destination of a moveOn
	int prevcX;			// Saved previous circle center x coordinate
	int prevcY;			// Saved previous circle center y coordinate
	int prevrX;			// Saved previous rectangle center x coordinate
	int prevrY;			// Saved previous rectangle center y coordinate

	/* Positon properties for derived class */
	int desiredPosX;	// Desired x coordinate position given to derived class
	int desiredPosY;	// Desired y coordinate position given to derived class

	/* Image buffers for visibility, */
	VGImage BackgroundBuffer;	// Image buffer containing background behind object
	VGImage UpdateBuffer;		// Image buffer containing static display elements to prevent re-draw
	VGImage MovementBuffer;		// Image buffer containing background behind movement path to prevent wiping behind moving object

protected:
	/* Constructor */
	TouchableObject(void);				// Sets up TouchableObject, sets properties to safe state

	/* Methods called by derived classes */
	bool getVisibility(void);			// Called by derived classes to determine visibility state
	bool getLPVisibility(void);			// Called by derived classes to determine previous visiblity state
	int getDesiredPosX(void);			// Called by derived classes to determine whether or not to move in the x direction
	int getDesiredPosY(void); 			// Called by derived classes to determine whether or not to move in the y direction
	void setCircular(void);				// Called by derived class to set touch area as circular
	void setCircleCenter(int, int);		// Called by derived class to set circular touch area center
	void setCircleRadius(int);			// Called by derived class to set circular touch area radius
	void setRectangular(void);			// Called by derived class to set touch area as rectangular
	void setRectWidthHeight(int, int);	// Called by derived class to set rectangular touch area size
	void setRectCenter(int, int);		// Called by derived class to set rectangular touch area bottom left corner

public:
	/* Control methods */
	void touchEnable(void);				// Enables touch functionality
	void touchDisable(void);			// Disables touch functionality
	void setVisible(void);				// Sets display object visible
	void setInvisible(void);			// Sets display object invisible

	/* Movement methods */
	void updatePosition(void);			// Called to update object position (handles moves)
	void moveTo(int, int, int);			// Moves object (delta X, delta Y, transition time in ms) (regular move)
	void moveOffRight(void);			// Moves object off of the screen to the right
	void moveOnRight(void);				// Moves object back onto the screen to its previous position from the right
	void moveOffLeft(void);				// Moves object off of the screen to the left
	void moveOnLeft(void);				// Moves object back onto the screen to its previous position from the left
	void moveOffBottom(void);			// Moves object off of the screen to the bottom
	void moveOnBottom(void);			// Moves object back onto the screen to its previous position from the bottom
	void moveOffTop(void);				// Moves object off of the screen to the top
	void moveOnTop(void);				// Moves object back onto the screen to its previous position from the top

	/* Touch methods */
	void updateTouch(touch_t);			// Provides touch data to object for touch processing
	bool isTouched(void);				// Returns if the object is currently being touched
	bool isHeld(void);					// Returns if the object is being touched and held  **** Need to define threshold ****
	bool isReleased(void);				// Returns if the object was released following a touch
	int getTouchDuration(void);			// Returns current touch duration (during and after touch)
};

#endif