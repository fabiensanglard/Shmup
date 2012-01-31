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
#include <windows.h>
#include <string.h>
#include <strsafe.h>

extern "C" {
#include "../src/dEngine.h"
#include "../src/commands.h"
#include "../src/timer.h"
#include "../src/menu.h"
}


#include "win32EGL.h"





void WIN_CheckError(char* errorHeader){

	DWORD errorCode;
	LPVOID lpMsgBuf;
    LPVOID lpDisplayBuf;


	errorCode = GetLastError();

	
	FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | 
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        errorCode,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR) &lpMsgBuf,
        0, NULL );
 

	
    lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT,(lstrlen((LPCTSTR)lpMsgBuf)+lstrlen((LPCTSTR)errorHeader)+40)*sizeof(TCHAR)); 
   
	
	StringCchPrintf((LPTSTR)lpDisplayBuf, 
	                  LocalSize(lpDisplayBuf) / sizeof(TCHAR),
					  TEXT("%s failed with error %d: %s"),
					  errorHeader, 
					  errorCode, 
					  lpMsgBuf); 
	

	printf("'%s'\n",(char*)lpDisplayBuf);
	

    LocalFree(lpMsgBuf);
    LocalFree(lpDisplayBuf);

}

#define KEYDOWN(vk_code)  ((GetAsyncKeyState(vk_code) & 0x8000) ? 1 : 0)
#define KEYUP(vk_code)  ((GetAsyncKeyState(vk_code) & 0x8000) ? 0 : 1)

void WIN_CheckInputs(void){}

void WIN_ReadInputs(){

	int numButtons;
	touch_t* currentTouchSet;

	if (engine.menuVisible)
	{
		numButtons = MENU_GetNumButtonsTouches();
		currentTouchSet = MENU_GetCurrentButtonTouches();
		
	}
	else 
	{
		numButtons = NUM_BUTTONS;
		currentTouchSet = touches;
	}

	int buttonPressed = KEYDOWN(VK_LBUTTON);
	if (!buttonPressed)
		return;

	//Get the mouse coordinates in screenspace.
	CURSORINFO pci ;
	pci.cbSize = sizeof(pci);
	BOOL success = GetCursorInfo(&pci);

	if (!success){
		WIN_CheckError("GetCursorInfo");
		return;
	}

	//Convert the screenspage to windowspace coordinates.
	success = ScreenToClient(WIN_GetHWND(),&pci.ptScreenPos);
	if (!success){
		WIN_CheckError("ScreenToClient");
		return;
	}

	printf("wc: %d,%d\n",pci.ptScreenPos.x,pci.ptScreenPos.y);
}


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	uchar engineParameters = 0;
	char cwd[256];
	WORD nBufferLength=256;
	char lpBuffer[256];

	Create_NativeWindow(hInstance, hPrevInstance, lpCmdLine, nCmdShow);

	//Create a console so we can see outputs.
	AllocConsole();
	freopen("conin$","r",stdin);
	freopen("conout$","w",stdout);
	freopen("conout$","w",stderr);
	HWND  consoleHandle = GetConsoleWindow();
	MoveWindow(consoleHandle,1,1,680,480,1);
	printf("[sys_win.c] Console initialized.\n");




	GetCurrentDirectoryA(nBufferLength,lpBuffer);
	memset(cwd,0,256);
	strcat(cwd,"RD=");
	strcat(cwd,lpBuffer);
	_putenv(cwd);

	memset(cwd,0,256);
	strcat(cwd,"WD=");
	strcat(cwd,lpBuffer);
	_putenv(cwd);

	engineParameters |= GL_11_RENDERER ;
	renderer.statsEnabled = 0;
	renderer.materialQuality = MATERIAL_QUALITY_LOW;

    renderer.resolution = WIN32_WINDOWS_SCALE;
    renderer.glBuffersDimensions[WIDTH] = WIN32_WINDOWS_WIDTH;
	renderer.glBuffersDimensions[HEIGHT] = WIN32_WINDOWS_HEIGHT;
	

	gameOn = 1;

	
	dEngine_Init();
	renderer.statsEnabled = 0;

	

	dEngine_InitDisplaySystem(engineParameters);


	/*
          The only complicated thing here is the time to sleep. timediff returned by the engine is telling us how long the frame should last.
		  Since a PC can render a frame in 1-2ms we need to substract frame hosting duration to timediff (either 16 or 17ms) and sleep for this amount of time.
	*/
	while(gameOn)
	{
		PumpWindowsMessages();

		// Check the state of the mouse and its position, may generate a touch_t if the 
		// left button is pressed.
		unsigned long startFrame = timeGetTime();


		WIN_ReadInputs();
		dEngine_HostFrame();
		EGLSwapBuffers();

		unsigned long endFrame = timeGetTime();

		unsigned long timeForFrame = endFrame - startFrame;
		
		int timeToSleep = timediff - timeForFrame;

		//printf("timeToSleep=%d\n",timeToSleep);
		// Game is clocked at 60Hz (timediff will be either 16 or 17, this value
		// comes from timer.c).
		if (timeToSleep > 0)
			Sleep(timeToSleep);
	}


	Destroy_NativeWindow();
	return 0;
}