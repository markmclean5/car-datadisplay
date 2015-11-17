 
using namespace std;
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <iostream>
#include <cmath>
#include "TouchableObject.h"

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
					if(touchStruct.abs_y >= yMin && touchStruct.abs_y <= yMax)
					{
						touched = true;
					}
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

void TouchableObject::move(int changeX, int changeY, int transTime)
{
//	if(isRectangular)
//	{
		desiredPosX = rX + changeX;
		desiredPosY = rY + changeY;
//	}
}

void TouchableObject::moveOffRight(void)
{
	if(!moveOffRt)
	{
		moveOffRt = true;
		if(isRectangular)
		{
			prevrX = rX;
			prevrY = rY;
		}
		else
		{
			prevcX = cX;
			prevcY = cY;
		}
	}
}

void TouchableObject::moveOnRight(void)
{
	if(!moveOnRt) moveOnRt = true;
}

void TouchableObject::updatePosition(void)
{
	if(moveOffRt)
	{
		if(isRectangular)
		{
			if (rX > 800 + rW)
			{
				moveOffRt = false;
			}
			else
			{
				desiredPosX = rX + 1;
			}
		}
		else
		{
			if (cX > 800 + cRad)
			{
				moveOffRt = false;
			}
			else
			{
				desiredPosX = cX + 1;
			}
		}
	}
	if(moveOnRt)
	{
		if(isRectangular)
		{
			if (rX == prevrX)
			{
				moveOnRt = false;
			}
			else
			{
				desiredPosX = rX - 1;
			}
		}
		else
		{
			if (cX == prevcX)
			{
				moveOnRt = false;
			}
			else
			{
				desiredPosX = cX + -1;
			}
		}
	}
}


int TouchableObject::getDesiredPosX(void)
{
	return desiredPosX;
}

int TouchableObject::getDesiredPosY(void)
{
	return desiredPosY;
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

	moveOffRt = false;
	moveOnRt = false;
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
	desiredPosX = cX;
	desiredPosY = cY;
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
	desiredPosX = rX;
	desiredPosY = rY;
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
}

void TouchableObject::setInvisible(void)
{
	lpVisible = visible;
	visible = false;
}


