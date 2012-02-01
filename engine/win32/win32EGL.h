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

#ifndef WIN32EGL
#define WIN32EGL


#include <GLES/egl.h>
#include <GLES/gl.h>
#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif

		void Create_NativeWindow(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow);
		void Destroy_NativeWindow(void);
		void EGLSwapBuffers(void);
		void PumpWindowsMessages(void);
		extern  int gameOn;
		HWND WIN_GetHWND();

#ifdef __cplusplus
}
#endif

#define WIN32_WINDOWS_WIDTH (320)
#define WIN32_WINDOWS_HEIGHT (480)

//#define WIN32_WINDOWS_WIDTH (768)
//#define WIN32_WINDOWS_HEIGHT (1024)



#endif
 