/*
	This file is part of SHMUP.

    SHMUP is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    SHMUP is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
*/    

#include "win32EGL.h"



#include <windows.h>
#include <TCHAR.h>
#include "../src/renderer.h"

 int gameOn =1;

#define	WINDOW_CLASS _T("PVRShellClass")
#define f2vt(x)				(x)
#define VERTTYPE		GLfloat
#define VERTTYPEENUM	GL_FLOAT
GLuint	ui32Vbo = 0; // Vertex buffer object handle

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		// Handles the close message when a user clicks the quit icon of the window
		case WM_CLOSE:
			gameOn = 0;
			PostQuitMessage(0);
			return 1;

		default:
			break;
	}

	// Calls the default window procedure for messages we did not handle
	return DefWindowProc(hWnd, message, wParam, lParam);
}

bool TestEGLError(HWND hWnd, char* pszLocation)
{
	
	//	eglGetError returns the last error that has happened using egl,
	//	not the status of the last called function. The user has to
	//	check after every single egl call or at least once every frame.
	
	EGLint iErr = eglGetError();
	if (iErr != EGL_SUCCESS)
	{
		TCHAR pszStr[256];
		_stprintf(pszStr, _T("%s failed (%d).\n"), pszLocation, iErr);
		MessageBox(hWnd, pszStr, _T("Error"), MB_OK|MB_ICONEXCLAMATION);
		return false;
	}

	return true;
}


EGLDisplay			eglDisplay	= 0;
EGLSurface			eglSurface	= 0;

HWND				hWnd	= 0;

void Create_NativeWindow(void)
{
	HWND hwndC = GetConsoleWindow() ;	
	HINSTANCE hInstance = (HINSTANCE)GetWindowLong( hwndC, GWL_HINSTANCE );

	HDC					hDC		= 0;
	

	// EGL variables
	EGLConfig			eglConfig	= 0;
	EGLContext			eglContext	= 0;
	NativeWindowType	eglWindow	= 0;
	EGLint				pi32ConfigAttribs[128];


	WNDCLASS sWC;
	sWC.style = CS_HREDRAW | CS_VREDRAW;
	sWC.lpfnWndProc = WndProc;
    sWC.cbClsExtra = 0;
    sWC.cbWndExtra = 0;
    sWC.hInstance = hInstance;
    sWC.hIcon = 0;
    sWC.hCursor = 0;
    sWC.lpszMenuName = 0;
	sWC.hbrBackground = (HBRUSH) GetStockObject(WHITE_BRUSH);
    sWC.lpszClassName = WINDOW_CLASS;
	unsigned int nWidth = renderWidth;
	unsigned int nHeight = renderHeight;

	ATOM registerClass = RegisterClass(&sWC);
	if (!registerClass)
	{
		MessageBox(0, _T("Failed to register the window class"), _T("Error"), MB_OK | MB_ICONEXCLAMATION);
		exit(0);
	}


	RECT	sRect;
	SetRect(&sRect, 0, 0, nWidth, nHeight);
	AdjustWindowRectEx(&sRect, WS_CAPTION | WS_SYSMENU, false, 0);
	hWnd = CreateWindow( WINDOW_CLASS, _T("Initialization"), WS_VISIBLE | WS_SYSMENU,0+100, 0+100, nWidth, nHeight, NULL, NULL, hInstance, NULL);

	eglWindow = hWnd;

	// Get the associated device context
	hDC = GetDC(hWnd);
	if (!hDC)
	{
		MessageBox(0, _T("Failed to create the device context"), _T("Error"), MB_OK|MB_ICONEXCLAMATION);
		exit(0);
	}




	eglDisplay = eglGetDisplay((NativeDisplayType)hDC);

    if(eglDisplay == EGL_NO_DISPLAY)
         eglDisplay = eglGetDisplay((NativeDisplayType) EGL_DEFAULT_DISPLAY);
	
	//	Step 2 - Initialize EGL.
	//	EGL has to be initialized with the display obtained in the
	//	previous step. We cannot use other EGL functions except
	//	eglGetDisplay and eglGetError before eglInitialize has been
	//	called.
	//	If we're not interested in the EGL version number we can just
	//	pass NULL for the second and third parameters.
	
	EGLint iMajorVersion, iMinorVersion;
	if (!eglInitialize(eglDisplay, &iMajorVersion, &iMinorVersion))
	{

		MessageBox(0, _T("eglInitialize() failed."), _T("Error"), MB_OK|MB_ICONEXCLAMATION);
		exit(0);
	}



	int i = 0;
	pi32ConfigAttribs[i++] = EGL_RED_SIZE;
	pi32ConfigAttribs[i++] = 5;
	pi32ConfigAttribs[i++] = EGL_GREEN_SIZE;
	pi32ConfigAttribs[i++] = 6;
	pi32ConfigAttribs[i++] = EGL_BLUE_SIZE;
	pi32ConfigAttribs[i++] = 5;
	pi32ConfigAttribs[i++] = EGL_ALPHA_SIZE;
	pi32ConfigAttribs[i++] = 0;
	pi32ConfigAttribs[i++] = EGL_SURFACE_TYPE;
	pi32ConfigAttribs[i++] = EGL_WINDOW_BIT;
	pi32ConfigAttribs[i++] = EGL_DEPTH_SIZE;
	pi32ConfigAttribs[i++] = 8;
	pi32ConfigAttribs[i++] = EGL_NONE;


	int iConfigs;
	if (!eglChooseConfig(eglDisplay, pi32ConfigAttribs, &eglConfig, 1, &iConfigs) || (iConfigs != 1))
	{

		MessageBox(0, _T("eglChooseConfig() failed."), _T("Error"), MB_OK|MB_ICONEXCLAMATION);
		exit(0);
	}


	eglSurface = eglCreateWindowSurface(eglDisplay, eglConfig, eglWindow, NULL);

    if(eglSurface == EGL_NO_SURFACE)
    {
        eglGetError(); // Clear error
        eglSurface = eglCreateWindowSurface(eglDisplay, eglConfig, NULL, NULL);
	}

	if (!TestEGLError(hWnd, "eglCreateWindowSurface"))
	{
		exit(0);
	}

	
	//	Step 6 - Create a context.
	//	EGL has to create a context for OpenGL ES. Our OpenGL ES resources
	//	like textures will only be valid inside this context
	//	(or shared contexts)
	
	eglContext = eglCreateContext(eglDisplay, eglConfig, NULL, NULL);
	if (!TestEGLError(hWnd, "eglCreateContext"))
	{
		exit(0);
	}


	
	//	Step 7 - Bind the context to the current thread and use our
	//	window surface for drawing and reading.
	//	Contexts are bound to a thread. This means you don't have to
	//	worry about other threads and processes interfering with your
	//	OpenGL ES application.
	//	We need to specify a surface that will be the target of all
	//	subsequent drawing operations, and one that will be the source
	//	of read operations. They can be the same surface.
	
	eglMakeCurrent(eglDisplay, eglSurface, eglSurface, eglContext);
	if (!TestEGLError(hWnd, "eglMakeCurrent"))
	{

		exit(0);
	}

	
	//	Step 8 - Draw something with OpenGL ES.
	//	At this point everything is initialized and we're ready to use
	//	OpenGL ES to draw something on the screen.
	

/*
	// We're going to draw a triangle to the screen so create a vertex buffer object for our triangle
	{
		// Interleaved vertex data
		VERTTYPE afVertices[] = {	f2vt(-0.4f),f2vt(-0.4f),f2vt(0.0f), // Position
									f2vt(1.0f),f2vt(1.0f),f2vt(0.66f),f2vt(1.0f), // Colour
									f2vt(+0.4f),f2vt(-0.4f),f2vt(0.0f),
									f2vt(1.0f),f2vt(1.0f),f2vt(0.66f),f2vt(1.0f),
									f2vt(0.0f),f2vt(0.4f),f2vt(0.0f),
									f2vt(1.0f),f2vt(1.0f),f2vt(0.66f),f2vt(1.0f)};

		// Generate the vertex buffer object (VBO)
		glGenBuffers(1, &ui32Vbo);

		// Bind the VBO so we can fill it with data
		glBindBuffer(GL_ARRAY_BUFFER, ui32Vbo);

		// Set the buffer's data
		unsigned int uiSize = 3 * (sizeof(VERTTYPE) * 7); // Calc afVertices size (3 vertices * stride (7 verttypes per vertex (3 pos + 4 colour)))
		glBufferData(GL_ARRAY_BUFFER, uiSize, afVertices, GL_STATIC_DRAW);
	}


	// Enable vertex arrays
		glEnableClientState(GL_VERTEX_ARRAY);

		
		// Set the vertex pointer.
		// param 1: Number of coordinates per vertex; must be 2, 3, or 4.
		//   param 2: GL_FIXED for CommonLite and GL_FLOAT for Common profile.
		//   param 3: Specifies the byte offset between consecutive vertexes. 
		//   param 4: Offset to the start of the first vertex into the VBO.
		 
		glVertexPointer(3, VERTTYPEENUM, sizeof(VERTTYPE) * 7, 0);

		// Set color data in the same way
		glEnableClientState(GL_COLOR_ARRAY);
		glColorPointer(4,VERTTYPEENUM,sizeof(VERTTYPE) * 7, (GLvoid*) (sizeof(VERTTYPE) * 3) );//The color starts after the 3 position values (x,y,z)

		
		//	Draws a non-indexed triangle array from the pointers previously given.
		//	param 1: Primitive type: GL_POINTS, GL_LINES, GL_TRIANGLE_STRIP, GL_TRIANGLE_FAN, GL_TRIANGLES, etc.
		//	param 2: The starting index in the array.
		//	param 3: The number of indices to be rendered.
		
		glDrawArrays(GL_TRIANGLES, 0, 3);

*/
		eglSwapBuffers(eglDisplay, eglSurface);
}



void EGLSwapBuffers(void)
{
	eglSwapBuffers(eglDisplay, eglSurface);
	FlushWindowsMessages();
}

void FlushWindowsMessages(void)
{
	MSG msg;
	PeekMessage(&msg, hWnd, NULL, NULL, PM_REMOVE);
	TranslateMessage(&msg);
	DispatchMessage(&msg);
}

#define KEYDOWN(vk_code)  ((GetAsyncKeyState(vk_code) & 0x8000) ? 1 : 0)
#define KEYUP(vk_code)  ((GetAsyncKeyState(vk_code) & 0x8000) ? 0 : 1)

void WIN_CheckInputs(void)
{
	
		
}



void Destroy_NativeWindow(void)
{
	eglMakeCurrent(eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
	eglTerminate(eglDisplay);
}