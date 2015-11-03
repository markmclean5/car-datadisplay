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
	if ((touch.fd = open("/dev/input/event0", O_RDONLY)) < 0) { 
		fprintf(stderr, "Error opening touch!\n");
		quitState = 1;
		return &quitState;
	}

	int mt_tracking_id = 0;
	bool btn_touch = false;
	int abs_x = 0;
	int abs_y = 0;

	while (1) {
		read(touch.fd, &touch.ev, sizeof(struct input_event)); // read file descriptor
		if(touch.ev.type == EV_SYN)
		{
			//std::cout << "EV_SYN TYPE" << std::endl;
			if(touch.ev.code == SYN_REPORT)
			{
				//std::cout << "SYN CODE SYN_REPORT" << std::endl;
				touch.mt_tracking_id = mt_tracking_id;
				touch.btn_touch = btn_touch;
				touch.abs_x = abs_x;
				touch.abs_y = abs_y;
			}
		}
		else if(touch.ev.type == EV_ABS)
		{
			//std::cout << "EV_ABS TYPE" << std::endl;
			if(touch.ev.code == ABS_MT_TRACKING_ID)
			{
				//std::cout << "ABS CODE MT TRACKING ID: " << touch.ev.value << std::endl;
				mt_tracking_id = touch.ev.value;
			}
			else if(touch.ev.code == ABS_X)
			{
				//std::cout << "ABS CODE ABS X: " << touch.ev.value <<std::endl;
				abs_x = touch.ev.value;
			}
			else if(touch.ev.code == ABS_Y)
			{
				//std::cout << "ABS CODE ABS Y: " << touch.ev.value << std::endl;
				abs_y = touch.ev.value;
			}
				
		}
		else if(touch.ev.type == EV_KEY)
		{
			//std::cout << "EV_KEY TYPE" << std::endl;
			if(touch.ev.code == BTN_TOUCH)
			{
				//std::cout << "KEY CODE ABS BTN_TOUCH: " << touch.ev.value << std::endl;
				btn_touch = touch.ev.value;
			}

		}
	}
}
