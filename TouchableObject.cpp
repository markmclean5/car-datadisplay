 
using namespace std;
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <iostream>
#include <cmath>
#include "TouchableObject.h"
#include <bcm2835.h>

/* Desired fade percentage getter - called by derived class */
int TouchableObject::getDesiredFadePercentage(void) {
	return fadePercentage;
}

/* Fade Method */
void TouchableObject::fade(int finalPercentage, int duration, string type){
	if(finalFadePercentage!=finalPercentage) {
		initialFadePercentage = fadePercentage;
		finalFadePercentage = finalPercentage;
		fadeDuration = duration;
		fadeType.assign(type);
		fadeStartTime = bcm2835_st_read();
	}
}

/* Move To method - move from current position to new position in given duration using move style */
void TouchableObject::moveTo(int finalX, int finalY, int dur, string type) {
	if(finalX != finalPosX || finalY != finalPosY) {
		moveStartX = cX;
		moveStartY = cY;	
		finalPosX = finalX;
		finalPosY = finalY;
		moveStartTime = bcm2835_st_read();
		moveDuration = dur;
		motionType.assign(type);
		moving = false;
	}
	else cout << "Move called which is already in progress" << endl;
}

/* Move method - move from current position to new position in given duration using move style */
void TouchableObject::move(int deltaX, int deltaY, int dur, string type) {
	if(moveStartX + deltaX != finalPosX || moveStartY + deltaY != finalPosY) {
		moveStartX = cX;
		moveStartY = cY;	
		finalPosX = cX + deltaX;
		finalPosY = cY + deltaY;
		moveStartTime = bcm2835_st_read();
		moveDuration = dur;
		motionType.assign(type);
		moving = false;
	}
	else cout << "Move called which is already in progress" << endl;
}

/* Update visuals method - updates movement position and animations */
void TouchableObject::updateVisuals(void) {
	// Manage Movement
	if(cX != finalPosX || cY != finalPosY) {
		moving = true;
		int totalPixDisp = sqrt(pow(finalPosX-moveStartX, 2) + pow(finalPosY-moveStartY, 2));
		float omega = (M_PI/2)/moveDuration;
		uint64_t currentTime = bcm2835_st_read();
		if(currentTime > (moveStartTime + moveDuration*1000)) {
			cX = finalPosX;
			cY = finalPosY;
		}
		else {
			int currentPixDisp = totalPixDisp*sin((omega/1000)*(currentTime - moveStartTime));
			double moveAngle = atan2(finalPosY - moveStartY, finalPosX - moveStartX);
			cX = moveStartX + currentPixDisp * cos(moveAngle);
			cY = moveStartY + currentPixDisp * sin(moveAngle);
		}
	}
	else moving = false;
	// Manage Fade
	if(fadePercentage != finalFadePercentage){
		int deltaFadePercentage = finalFadePercentage - initialFadePercentage;
		float omega = (M_PI/2)/fadeDuration;
		uint64_t currentTime = bcm2835_st_read();
		if(currentTime > fadeStartTime + fadeDuration * 1000)
			fadePercentage = finalFadePercentage;
		else {
			fadePercentage = initialFadePercentage + deltaFadePercentage*sin((omega/1000)*(currentTime-fadeStartTime));
		}
	}
}

/* Update Function */
void TouchableObject::updateTouch(touch_t touchStruct) {
	if(touchEnabled) {
		if(touchStruct.btn_touch) {
			if(isRectangular) {			// Figure out if touch falls within rectangle bounds
				int xMin = cX - rW/2;
				int xMax = cX + rW/2;
				int yMin = cY - rH/2;
				int yMax = cY + rH/2;
				if(touchStruct.abs_x >= xMin && touchStruct.abs_x <= xMax) {
					if(touchStruct.abs_y >= yMin && touchStruct.abs_y <= yMax) touched = true;		
					else touched = false;
				}
			}
			else {					// Figure out if touch falls within circle bounds
				float deltaX = touchStruct.abs_x - cX;
				float deltaY = touchStruct.abs_y - cY;
				float touchDist = sqrt(pow(deltaX, 2) + pow(deltaY, 2));
				if (touchDist <= cRad) touched = true;
				else touched = false;
			}
		}
		else touched = false;
	}
	pressProcessing();
}

void TouchableObject::pressProcessing(void) {
	uint64_t currentTime = bcm2835_st_read();
	// Start a press if the button is touched and not being maintained for debounce duration
	if(!pressed && isTouched() && !inDebounce) {
		inDebounce = true;
		pressed = true;
		pressRead = false;
		pressStartTime = bcm2835_st_read();
		debounceFinishTime = pressStartTime + 1000*debounceDuration;
	}
	// Finish a press if debounce duration is reached
	if(inDebounce && (currentTime >= debounceFinishTime)) {
		pressed = false;
		inDebounce = false;
	}
	// Finish a press once the press has been read
	if(pressed && pressRead) {
		pressed = false;
	}
}

/* TouchableObject Setters */
void TouchableObject::setPressDebounce(int duration) {
	debounceDuration = duration;
}
bool TouchableObject::isPressed(void) {
	if(pressed) pressRead = true;
	return pressed;
}
bool TouchableObject::isMoving(void) {
	return moving;
}
int TouchableObject::getDesiredPosX(void) {
	return cX;
}
int TouchableObject::getDesiredPosY(void) {
	return cY;

}
bool TouchableObject::isTouched(void) {
	return touched;
}

/* Constructor */
TouchableObject::TouchableObject(void) {
	isRectangular = true;
	cX = 0;
	cY = 0;
	cRad = 0;
	rW = 0;
	rH = 0;

	touchEnabled = false;
	visible = false;
	lpVisible = false;
	touched = false;
	finalPosX = -1;
	finalPosY = -1;
	movingOffRight = false;
	movingOnRight = false;
	pressed = false;
	debounceDuration = 0;
	inDebounce = false;
	fadePercentage = 0;
	finalFadePercentage = 0;
}

/* Visibility getters */
bool TouchableObject::getVisibility(void) {
	return visible;
}
bool TouchableObject::getLPVisibility(void) {
	return lpVisible;
}

/* Rectangular setters */
void TouchableObject::setCircular(void) {
	isRectangular = false;
}

void TouchableObject::setCircleCenter(int x, int y) {
	cX = x;
	cY = y;
	finalPosX = cX; 
	finalPosY = cY;
}
void TouchableObject::setCircleRadius(int rad) {
	cRad = rad;
}

/* Rectangular setters */
void TouchableObject::setRectangular(void) {
	isRectangular = true;
}
void TouchableObject::setRectWidthHeight(int w, int h) {
	rW = w;
	rH = h;
}
void TouchableObject::setRectCenter(int x, int y) {
	cX = x;
	cY = y;
	finalPosX = cX; 
	finalPosY = cY;
}

/* Touch control */
void TouchableObject::touchEnable(void) {
	touchEnabled = true;
}
void TouchableObject::touchDisable(void) {
	touchEnabled = false;
}

/* Visibility control */
void TouchableObject::setVisible(void) {
	lpVisible = visible;
	visible = true;
	alpha = 1.0;
}
void TouchableObject::setInvisible(void) {
	lpVisible = visible;
	visible = false;
}