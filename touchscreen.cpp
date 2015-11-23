using namespace std;

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>

#include "VG/openvg.h"
#include "VG/vgu.h"

#include <linux/input.h>
#include <fcntl.h>
#include <pthread.h>

#include "touchscreen.h"
#include "project.h"

/* touchinit starts the touch event thread */
int touchinit(int w, int h)
{
	pthread_t inputThread;
	return pthread_create(&inputThread, NULL, &eventThread, NULL);
}

/* evenThread reads from the touch input file */
void *eventThread(void *arg)
{
	// Open touch driver file descriptor
	if ((threadTouch.fd = open("/dev/input/event0", O_RDONLY)) < 0) { 
		fprintf(stderr, "Error opening touch!\n");
		quitState = 1;
		return &quitState;
	}

	int mt_tracking_id = 0;
	bool btn_touch = false;
	int abs_x = 0;
	int abs_y = 0;

	while (1) {
		read(threadTouch.fd, &threadTouch.ev, sizeof(struct input_event)); // read file descriptor
		if(threadTouch.ev.type == EV_SYN)
		{
			//std::cout << "EV_SYN TYPE" << std::endl;
			if(threadTouch.ev.code == SYN_REPORT)
			{
				//std::cout << "SYN CODE SYN_REPORT" << std::endl;
				threadTouch.mt_tracking_id = mt_tracking_id;
				threadTouch.btn_touch = btn_touch;
				threadTouch.abs_x = abs_x;
				threadTouch.abs_y = abs_y;
			}
		}
		else if(threadTouch.ev.type == EV_ABS)
		{
			//std::cout << "EV_ABS TYPE" << std::endl;
			if(threadTouch.ev.code == ABS_MT_TRACKING_ID)
			{
				//std::cout << "ABS CODE MT TRACKING ID: " << threadTouch.ev.value << std::endl;
				mt_tracking_id = threadTouch.ev.value;
			}
			else if(threadTouch.ev.code == ABS_X)
			{
				//std::cout << "ABS CODE ABS X: " << threadTouch.ev.value <<std::endl;
				abs_x = threadTouch.ev.value;
			}
			else if(threadTouch.ev.code == ABS_Y)
			{
				//std::cout << "ABS CODE ABS Y: " << threadTouch.ev.value << std::endl;
				abs_y = 480 - threadTouch.ev.value;
			}	
		}
		else if(threadTouch.ev.type == EV_KEY)
		{
			//std::cout << "EV_KEY TYPE" << std::endl;
			if(threadTouch.ev.code == BTN_TOUCH)
			{
				//std::cout << "KEY CODE ABS BTN_TOUCH: " << threadTouch.ev.value << std::endl;
				btn_touch = threadTouch.ev.value;
			}
		}
	}
}
