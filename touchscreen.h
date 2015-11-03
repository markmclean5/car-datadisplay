#ifndef TOUCHSCREEN_H
#define TOUCHSCREEN_H
/**************************************************************************************************************
 * Touchscreen related functions
 * Responsible for detecting touch input data and populating touch_t structure for main thread
 *
 * Creates inputThread - event thread which reads /dev/input/event0 (touchscreen device)
 * 
 * inputThread: populates touch (touch_t structure) using input_event data.
 *				Updates structure upon each EV_SYN type SYN_REPORT code.
 * 
 **************************************************************************************************************/

/* References - input events */
// https://www.kernel.org/doc/Documentation/input/event-codes.txt
// https://www.kernel.org/doc/Documentation/input/multi-touch-protocol.txt

/* References - timeval struct */
// http://man7.org/linux/man-pages/man2/gettimeofday.2.html

/* Mouse state structure */
typedef struct {
	int fd;						// file descriptor for reading input_events
	struct input_event ev;		// input_event structure (timeval time structure, type, code, value)
	int mt_tracking_id;			// populated from EV_ABS , ABS_MT_TRACKING_ID
	bool btn_touch;				// populated from EV_KEY, BTN_TOUCH
	int abs_x;					// populated from EV_ABS, ABS_X
	int abs_y;					// populated from EV_ABS, ABS_Y
} touch_t;

extern touch_t touch;			// global mouse state
extern int quitState;			// get rid of this eventually?

/* mouseinit starts the mouse event thread */
int touchinit(int, int);

/* eventThread - thread created for reading inputEvent */
void *eventThread(void*);							
 

 #endif 