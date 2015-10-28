#ifndef TOUCHSCREEN_H
#define TOUCHSCREEN_H

/**************************************************************************************************************
 * Touchscreen related functions
 * Responsible for processing touch input data.
 * 
 *
 * Provides:	
 *
 **************************************************************************************************************/


// mouseinit starts the mouse event thread
int mouseinit(int, int);
void *eventThread(void*);							
 #endif