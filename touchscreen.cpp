using namespace std;

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "VG/openvg.h"
#include "VG/vgu.h"

#include <linux/input.h>
#include <fcntl.h>
#include <pthread.h>

#include "touchscreen.h"
#include "project.h"


// mouseinit starts the mouse event thread
int mouseinit(int w, int h) {
	pthread_t inputThread;
	return pthread_create(&inputThread, NULL, &eventThread, NULL);
}



// evenThread reads from the mouse input file
void *eventThread(void *arg) {
	// Open mouse driver
	if ((mouse.fd = open("/dev/input/event0", O_RDONLY)) < 0) {
		fprintf(stderr, "Error opening Mouse!\n");
		quitState = 1;
		return &quitState;
	}

	bool touchStarted = false;
	bool touchXRead = false;
	bool touchYRead = false;

	while (1) {
		read(mouse.fd, &mouse.ev, sizeof(struct input_event));
		printf("Event read: [%4.0f,%4.0f]\n",mouse.x,mouse.y);

		if(touchStarted){
			if (mouse.ev.type == EV_ABS)
			{
				if (mouse.ev.code == ABS_X)
				{
					mouse.x = (VGfloat) mouse.ev.value;
					if(touchStarted) touchXRead = true;
				}
				if (mouse.ev.code == ABS_Y)
				{
					mouse.y = (VGfloat) mouse.ev.value;
					if(touchStarted) touchYRead = true; 
				}
			}
		}
		
		if(touchStarted && touchXRead && touchYRead)
		{
			printf("Processed Touch position: [%4.0f,%4.0f]\n\n",mouse.x,mouse.y);
		}

		if(mouse.ev.type == EV_KEY)
		{
			if(mouse.ev.code == BTN_TOUCH)
			{
				printf("Time Stamp:%ld - ", mouse.ev.time.tv_usec);
				if(mouse.ev.value == 1) 
				{
					printf("Touch Started: \n");
					touchStarted = true;
				}
				if(mouse.ev.value == 0)
				{
					printf("Touch Completed: \n");
					touchStarted = false;
					touchXRead = false;
					touchYRead = false;
				}
			}
		}


/*		if (mouse.ev.type == EV_KEY) {
			//printf("Time Stamp:%d - type %d, code %d, value %d\n",
			//      mouse.ev.time.tv_usec,mouse.ev.type,mouse.ev.code,mouse.ev.value);
			if (mouse.ev.code == BTN_LEFT) {
				mouse.left = 1;
				//   printf("Left button\n");
				left_count++;
				// printf("User Quit\n");
				// quitState = 1;
				// return &quitState;  //Left mouse to quit
			}
			if (mouse.ev.code == BTN_RIGHT) {
				mouse.right = 1;
				//  printf("Right button\n");
			}
		}
*/
	}
}
