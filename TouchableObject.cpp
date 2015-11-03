
using namespace std;
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <iostream>
#include <cmath>
#include "TouchableObject.h"

/* Update Function */
void::TouchableObject::updateTouch(touch_t touchStruct)
{
	if(touchEnabled)
	{
		if(touchStruct.btn_touch)
		{
			if(isRectangular)		// Figure out if touch falls within rectangle bounds
			{
				int xMin = rblX;
				int xMax = rblX + rW;
				int yMax = rblY;
				int yMin = rblY - rH;

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
	rblX = 0;
	rblY = 0;
	touchEnabled = false;
	visible = false;
	lpVisible = false;
	touched = false;
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

void TouchableObject::setRectBottomLeft(int x, int y)
{
	rblX = x;
	rblY = y;
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


