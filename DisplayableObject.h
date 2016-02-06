#ifndef DISPLAYABLEOBJECT_H
#define DISPLAYABLEOBJECT_H

#include "EGL/egl.h"		// EGL for pbuffers


class DisplayableObject{
private:
	

	/**************************************************************************************************************
 	* Pixel buffer: 
 	* References:
 	* 	EGL Overview: https://www.khronos.org/registry/egl/sdk/docs/man/html/eglIntro.xhtml
 	* 	Creating pbuffer: https://www.khronos.org/registry/egl/sdk/docs/man/html/eglCreatePbufferFromClientBuffer.xhtml
 	**************************************************************************************************************/		

	// Display
	EGLDisplay display;				// EGL display connection (eglGetCurrentDisplay)
	EGLContext renderingContext;	// Rendering context (eglGetCurrentContext)
	EGLSurface displaySurface;		// Display surface (eglGetCurrentSurface)

	EGLint num_config;				// Number of EGLConfigs returned (eglChooseConfig)
	EGLBoolean result;				// Boolean used to store result of EGL calls
	EGLConfig config;				// Frame buffer configuration (eglChooseConfig)

public:
	DisplayableObject(void);			// DisplayableObject constructor

	// Initialize pixel buffer surface of given pixel size (width & height) using provided VGImage as the color buffer
	EGLSurface createBufferSurface(int, int, VGImage*);

	void switchToBufferSurface(EGLSurface);
	void switchToDisplaySurface(void);

	void drawBuffer(int, int, int, int, VGImage);


	
};


#endif




