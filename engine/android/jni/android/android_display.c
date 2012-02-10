#include "android_display.h"

EGLDisplay engineDisplay ;
EGLContext engineContext ;
EGLSurface engineSurface ;
ANativeWindow* window=0;

#include "../../../src/dEngine.h"
#include "../../../src/log.h"

/**
 * Initialize an EGL context for the current display.
 */
int engine_init_display(void) {

	uchar engineParameters = 0;


	// initialize OpenGL ES and EGL

	/*
	 * Here specify the attributes of the desired configuration.
	 * Below, we select an EGLConfig with at least 8 bits per color
	 * component compatible with on-screen windows
	 */
	const EGLint attribs[] = {
			EGL_SURFACE_TYPE ,/*=*/ EGL_WINDOW_BIT,
			EGL_BLUE_SIZE    ,/*=*/ 8,
			EGL_GREEN_SIZE   ,/*=*/ 8,
			EGL_RED_SIZE     ,/*=*/ 8,
            EGL_DEPTH_SIZE   ,/*=*/ 8,
			EGL_NONE
	};
	EGLint w, h, dummy, format;
	EGLint numConfigs;
	EGLConfig config;
	EGLSurface surface;
	EGLContext context;

	EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);

	eglInitialize(display, 0, 0);

	/* Here, the application chooses the configuration it desires. In this
	 * sample, we have a very simplified selection process, where we pick
	 * the first EGLConfig that matches our criteria */
	eglChooseConfig(display, attribs, &config, 1, &numConfigs);

	/* EGL_NATIVE_VISUAL_ID is an attribute of the EGLConfig that is
	 * guaranteed to be accepted by ANativeWindow_setBuffersGeometry().
	 * As soon as we picked a EGLConfig, we can safely reconfigure the
	 * ANativeWindow buffers to match, using EGL_NATIVE_VISUAL_ID. */
	eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format);

	ANativeWindow_setBuffersGeometry(window, 0, 0, format);

	surface = eglCreateWindowSurface(display, config, window, NULL);
	context = eglCreateContext(display, config, NULL, NULL);

	if (eglMakeCurrent(display, surface, surface, context) == EGL_FALSE) {
		Log_Printf("Unable to eglMakeCurrent");
		return -1;
	}

	eglQuerySurface(display, surface, EGL_WIDTH, &w);
	eglQuerySurface(display, surface, EGL_HEIGHT, &h);

	Log_Printf("Rendering surface is %dx%d.\n",w,h);

	engineDisplay = display;
	engineContext = context;
	engineSurface = surface;


	engineParameters |= GL_11_RENDERER ;


	renderer.glBuffersDimensions[WIDTH] = w;
	renderer.glBuffersDimensions[HEIGHT] = h;

	IO_Init();

  	dEngine_InitDisplaySystem(engineParameters);

    renderer.props |= PROP_FOG ;

	return 0;
}


 void engine_term_display(void) {

	if (engineDisplay != EGL_NO_DISPLAY)
	{
		eglMakeCurrent(engineDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);

		if (engineContext != EGL_NO_CONTEXT)
			eglDestroyContext(engineDisplay, engineContext);

		if (engineSurface != EGL_NO_SURFACE)
			eglDestroySurface(engineDisplay, engineSurface);

		eglTerminate(engineDisplay);
	}

	engineDisplay = EGL_NO_DISPLAY;
	engineContext = EGL_NO_CONTEXT;
	engineSurface = EGL_NO_SURFACE;
}




/**
 * Just the current frame in the display.
 */
 void engine_draw_frame(void) {

	 // Do we have somewhere to draw ?
	if (engineDisplay == NULL)
	{
		Log_Printf("[engine_draw_frame] Cannot draw.\n");
		return;
	}

	Log_Printf("[engine_draw_frame] Drawing.\n");
	dEngine_HostFrame();

	eglSwapBuffers(engineDisplay, engineSurface);
}


