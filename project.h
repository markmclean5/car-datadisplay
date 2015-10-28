#ifndef PROJECT_H
#define PROJECT_H

/**************************************************************************************************************
 * Touchscreen related functions
 * Responsible for processing touch input data.
 * 
 *
 * Provides:	
 *
 **************************************************************************************************************/

// Mouse state structure
typedef struct {
	int fd;
	struct input_event ev;
	VGfloat x, y;
	int left, middle, right;
	int max_x, max_y;
} mouse_t;

mouse_t mouse;			// global mouse state

int quitState = 0;

 #endif