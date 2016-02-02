#ifndef PROJECT_H
#define PROJECT_H

#include "EGL/egl.h"	// EGL for pbuffers


/**************************************************************************************************************
 * Main project header file
 **************************************************************************************************************/

/**************************************************************************************************************
 * Off-screen buffer: 
 * References:
 * 	EGL Overview: https://www.khronos.org/registry/egl/sdk/docs/man/html/eglIntro.xhtml
 * 	Creating pbuffer: https://www.khronos.org/registry/egl/sdk/docs/man/html/eglCreatePbufferFromClientBuffer.xhtml
 **************************************************************************************************************/		

// Display
EGLDisplay display;				// EGL display connection (eglGetCurrentDisplay)
EGLContext renderingContext;	// Rendering context (eglGetCurrentContext)
EGLSurface displaySurface;		// Display surface (eglGetCurrentSurface)

// Buffer
EGLSurface bufferSurface;		// Pixel buffer surface
VGImage bufferImage;			// VGImage color buffer for pixel buffer surface 

// Initialize pixel buffer surface of given pixel size (width & height) using provided VGImage as the color buffer
EGLSurface createBufferSurface(int, int, VGImage);











#endif