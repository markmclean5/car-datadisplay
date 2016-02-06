/********************************/
/*	DisplayableObject Class		*/
/********************************/
#include <iostream>
#include "VG/openvg.h"		//
#include "VG/vgu.h"			//
#include "fontinfo.h"		// OpenVG
#include "shapes.h"			//
#include "DisplayableObject.h"
#include "EGL/egl.h"		// EGL for pbuffers
#include <stdio.h>
#include <fstream>
#include <algorithm>
#include <stdlib.h>

using namespace std;

/* DisplayableObject Constructor: Determines EGL display connection, display surface, and rendering context */
DisplayableObject::DisplayableObject(void) {
	display = eglGetCurrentDisplay();
	if(display == EGL_NO_DISPLAY) cout << "Failed to get current display" << endl;
	static const EGLint attribute_list[] = {
		EGL_RED_SIZE, 8,
		EGL_GREEN_SIZE, 8,
		EGL_BLUE_SIZE, 8,
		EGL_ALPHA_SIZE, 8,
		EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
		EGL_NONE
	};
	result = eglChooseConfig(display, attribute_list, &config, 1, &num_config);
	if(result == EGL_FALSE) cout << "Failed to choose config" << endl;
	displaySurface = eglGetCurrentSurface(EGL_DRAW);
	if(displaySurface == EGL_NO_SURFACE) cout << "Failed to get current surface" << endl;
	renderingContext = eglGetCurrentContext();
	if(renderingContext == EGL_NO_CONTEXT) cout << "Failed to get current context" << endl;
}


/* Create pixel buffer surface */
EGLSurface DisplayableObject::createBufferSurface(int surfaceWidth, int surfaceHeight, VGImage * colorBuffer) {
	*colorBuffer = vgCreateImage(VG_sABGR_8888, surfaceWidth, surfaceHeight, VG_IMAGE_QUALITY_BETTER);
	static const EGLint surfAttr[] = {
		EGL_HEIGHT, surfaceHeight,
		EGL_WIDTH, surfaceWidth,
		EGL_NONE
	};
	EGLSurface bufferSurface = eglCreatePbufferFromClientBuffer (display, EGL_OPENVG_IMAGE, (EGLClientBuffer) *colorBuffer, config, surfAttr);
	if(bufferSurface == EGL_NO_SURFACE) cout << "Failed to create pbuffer surface" << endl;
	switchToBufferSurface(bufferSurface);
	float surfaceBackgroundColor[4];
	RGBA(0, 0, 0, 0, surfaceBackgroundColor);
	vgSetfv(VG_CLEAR_COLOR, 4, surfaceBackgroundColor);
	vgClear(0, 0, surfaceWidth, surfaceHeight);
	switchToDisplaySurface();
	return bufferSurface;
}

/* Switch to pixel buffer surface */
void DisplayableObject::switchToBufferSurface(EGLSurface bufferSurface) {
	result = eglMakeCurrent(display, bufferSurface, bufferSurface, renderingContext);
	if(result == EGL_FALSE) cout << "Failed to make buffer surface current" << endl;
}

/* Switch to display surface */
void DisplayableObject::switchToDisplaySurface(void) {
	result = eglMakeCurrent(display, displaySurface, displaySurface, renderingContext);
	if(result == EGL_FALSE) cout << "Failed to make display surface current" << endl;
}


