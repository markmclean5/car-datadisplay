 
using namespace std;
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <iostream>
#include <cmath>
#include "TouchableObject.h"
#include <bcm2835.h>

void TouchableObject::move(int finalX, int finalY, int dur, string type)
{
	cout << "Move called" << endl;
	if(finalX != finalPosX || finalY != finalPosY)
	{
		if(isRectangular)
		{
			moveStartRX = rX;
			moveStartRY = rY;	
		}
		else
		{
			moveStartCX = cX;
			moveStartCY = cY;		
		}
		cout << "Move: Setting new final position" << endl;
		finalPosX = finalX;
		finalPosY = finalY;


		int movementBufferWidth = abs(finalPosX-moveStartRX)+rW;
		int movementBufferHeight = abs(finalPosY-moveStartRY)+rH;
		MovementBuffer = vgCreateImage(VG_sABGR_8888, movementBufferWidth, movementBufferHeight, VG_IMAGE_QUALITY_BETTER);

		int sx, sy;		// Screen pixels to be placed in 0,0 (start) of buffer
		if(finalPosX-moveStartRX >= 0) sx = moveStartRX-rW/2;	// Moving right
		else sx = finalPosX+rW/2;								// Moving left
		if(finalPosY-moveStartRY >= 0) sy = moveStartRY-rH/2;	// Moving up
		else sy = finalPosY+rH/2;								// Moving left
		vgGetPixels(MovementBuffer, 0, 0, sx, sy, movementBufferWidth, movementBufferHeight);


		moveStartTime = bcm2835_st_read();
		moveDuration = dur;
		motionType.assign(type);
	}
	else cout << "Move called - already in progress" << endl;
}

void TouchableObject::updateVisuals(void)
{
	if(isRectangular)
	{
		if(rX != finalPosX || rY != finalPosY)
		{
			cout << "Inside updateVisuals: position change, move is necessary" << endl;
			int totalPixDisp = sqrt(pow(finalPosX-moveStartRX, 2) + pow(finalPosY-moveStartRY, 2));
			float omega = (M_PI/2)/moveDuration;
			uint64_t currentTime = bcm2835_st_read();
			if(currentTime > (moveStartTime + moveDuration*1000))
			{
				cout << "Overshot time!!" << endl;
				//currentTime = moveStartTime + moveDuration*1000;
				rX = finalPosX;
				rY = finalPosY;
			}
			else
			{
				int currentPixDisp = totalPixDisp*sin((omega/1000)*(currentTime - moveStartTime));
				double moveAngle = atan2(finalPosY - moveStartRY, finalPosX - moveStartRX);
				rX = moveStartRX + currentPixDisp * cos(moveAngle);
				rY = moveStartRY + currentPixDisp * sin(moveAngle);
			}
		}
	}
}

/* Update Function */
void TouchableObject::updateTouch(touch_t touchStruct)
{
	if(touchEnabled)
	{
		if(touchStruct.btn_touch)
		{
			if(isRectangular)		// Figure out if touch falls within rectangle bounds
			{
				int xMin = rX - rW/2;
				int xMax = rX + rW/2;
				int yMin = rY - rH/2;
				int yMax = rY + rH/2;
				if(touchStruct.abs_x >= xMin && touchStruct.abs_x <= xMax)
				{
					if(touchStruct.abs_y >= yMin && touchStruct.abs_y <= yMax) touched = true;
					else touched = false;
				}
			}
			else					// Figure out if touch falls within circle bounds
			{
				float deltaX = touchStruct.abs_x - cX;
				float deltaY = touchStruct.abs_y - cY;
				float touchDist = sqrt(pow(deltaX, 2) + pow(deltaY, 2));
				if (touchDist <= cRad) touched = true;
				else touched = false;
			}
		}
		else touched = false;
	}
}

int TouchableObject::getDesiredPosX(void)
{
	if(isRectangular) return rX;
	else return cX;
}

int TouchableObject::getDesiredPosY(void)
{
	if(isRectangular) return rY;
	else return cY;
}

float TouchableObject::getDesiredAlpha(void)
{
	return alpha;
}

bool TouchableObject::isTouched(void)
{
	return touched;
}

/* Constructor */
TouchableObject::TouchableObject(void)
{
	isRectangular = true;
	cX = 0;
	cY = 0;
	cRad = 0;
	rW = 0;
	rH = 0;
	rX = 0;
	rY = 0;

	touchEnabled = false;
	visible = false;
	lpVisible = false;
	touched = false;

	finalPosX = -1;
	finalPosY = -1;

	movingOffRight = false;
	movingOnRight = false;
}

/* Visibility getters */
bool TouchableObject::getVisibility(void)
{
	return visible;
}

bool TouchableObject::getLPVisibility(void)
{
	return lpVisible;
}

/* Rectangular setters */
void TouchableObject::setCircular(void)
{
	isRectangular = false;
}

void TouchableObject::setCircleCenter(int x, int y)
{
	cX = x;
	cY = y;
}

void TouchableObject::setCircleRadius(int rad)
{
	cRad = rad;
}

/* Rectangular setters */
void TouchableObject::setRectangular(void)
{
	isRectangular = true;
}

void TouchableObject::setRectWidthHeight(int w, int h)
{
	rW = w;
	rH = h;
}

void TouchableObject::setRectCenter(int x, int y)
{
	rX = x;
	rY = y;
	finalPosX = rX; 
	finalPosY = rY;
}

/* Touch control */
void TouchableObject::touchEnable(void)
{
	touchEnabled = true;
}

void TouchableObject::touchDisable(void)
{
	touchEnabled = false;
}


/* Visibility control */
void TouchableObject::setVisible(void)
{
	lpVisible = visible;
	visible = true;
	alpha = 1.0;
}

void TouchableObject::setInvisible(void)
{
	lpVisible = visible;
	visible = false;
}
