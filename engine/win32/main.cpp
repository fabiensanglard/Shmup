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
 
extern "C" {
#include "../src/dEngine.h"
#include "../src/commands.h"
#include "../src/timer.h"
}


#include "win32EGL.h"



void WIN_SwapRenderBuffers()
{
	EGLSwapBuffers();
	FlushWindowsMessages();
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

    renderer.resolution = 1;
    renderer.glBuffersDimensions[WIDTH] = 320;
	renderer.glBuffersDimensions[HEIGHT] = 480;
	

	gameOn = 1;

	
	dEngine_Init();
	renderer.statsEnabled = 0;

	

	dEngine_InitDisplaySystem(engineParameters);

	
	

	while(gameOn)
	{
		//commands[0].time = simulationTime;
		dEngine_HostFrame();


		WIN_SwapRenderBuffers();
	}


	Destroy_NativeWindow();
	return 0;
}