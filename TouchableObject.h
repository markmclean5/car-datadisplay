#ifndef TOUCHABLEOBJECT_H
#define TOUCHABLEOBJECT_H 

#include <linux/input.h>
#include "touchscreen.h"

#include "VG/openvg.h"		//
#include "VG/vgu.h"			//

using namespace std;
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
	int cX;				// Circle center x coordinate (current location, updated each position update)
	int cY;				// Circle center y coordinate (current location, updated each position update)
	int cRad;			// Circle radius
	/* Rectangular properties */
	int rW;				// Rectangle width
	int rH;				// Rectangle height
	int rX;				// Rectangle center x coordinate (current location, updated each position update)
	int rY;				// Rectangle center y coordinate (current location)

	float alpha;		// Current alpha

	/* Touch processing properties */
	bool touchEnabled;	// Enables touch processing
	bool touched;		// Current touch state

	/* Press processing properties */
	bool pressDebounceEnabled;
	uint64_t pressStartTime;
	uint64_t debounceFinishTime;
	int debounceDuration;
	bool pressed;
	bool pressRead;
	bool inDebounce;

	/* Visibility properties */
	bool visible;		// Current visibility state
	bool lpVisible;		// Previous visibility state

	/* Movement Properties */
	bool moving;


	bool movingOffRight;	// In process of moving object off of the screen to the right
	bool movingOnRight;		// In process of moving object onto the screen from the right
	bool movingOffLeft;		// In process of moving object off of the screen to the left
	bool movingOnLeft;		// In process of moving object onto the screen from the left
	bool movingOffBottom;	// In process of moving object off of the screen to the bottom
	bool movingOnBotttom;	// In process of moving object onto the screen from the bottom
	bool movingOffTop;		// In process of moving object off of the screen to the top
	bool movingOnTop;		// In process of moving object onto the screen from the top

	uint64_t moveStartTime;		// Time at the start of a move
	int moveDuration;			// Duration of move (milliseconds)


	float initialAlpha;			// Initial alpha
	float finalAlpha;			// Final alpha p
	string motionType;			// Type of movement (linear)


	int fadePercentage;
	int finalFadePercentage;
	int initialFadePercentage;
	int fadeDuration;
	uint64_t fadeStartTime;
	string fadeType;

	void pressProcessing(void);	// Press processing

protected:
	/* Image buffers for visibility, accessed directly by derived class*/
	VGImage BackgroundBuffer;	// Image buffer containing background behind object
	VGImage UpdateBuffer;		// Image buffer containing static display elements to prevent re-draw
	VGImage MovementBuffer;		// Image buffer containing background behind movement path to prevent wiping behind moving object
	
	/* Stored position properties, used in current positon calculation in move and by derived class for buffer re-draw */
	int moveStartX;			// Saved previous center x coordinate
	int moveStartY;			// Saved previous center y coordinate
	int finalPosX;				// Final desired center x coordinate of move
	int finalPosY;				// Final desired center y coordinate of move

	/* Constructor */
	TouchableObject(void);				// Sets up TouchableObject, sets properties to safe state

	/* Methods called by derived classes */
	bool getVisibility(void);			// Called by derived classes to determine visibility state
	bool getLPVisibility(void);			// Called by derived classes to determine previous visiblity state
	int getDesiredPosX(void);			// Called by derived classes to determine X position for TouchableObject
	int getDesiredPosY(void); 			// Called by derived classes to determine Y position for TouchableObject
	int getDesiredFadePercentage(void);	// Called by derived classes to determine fade percentage to apply to alpha values
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
	void updateVisuals(void);			// Called to update object visuals (handles moves and fades)
	bool isMoving(void);				// Called to determine if an object is in motion

	void move(int, int, int, string);	// Moves object (new X, new Y, duration (milliseconds), motion type string)
	void fade(int, int, string);		// Fades object (final desired fade percentage, duration (milliseconds), fade type string)
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

	void setPressDebounce(int);
	bool isPressed(void);

};

#endif