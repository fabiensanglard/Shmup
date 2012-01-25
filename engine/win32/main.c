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
#include "../src/dEngine.h"
#include "../src/commands.h"
#include "../src/timer.h"
#include <IL/il.h>
#pragma comment(lib, "winmm.lib")
#include "win32EGL.h"


void SND_InitSoundTrack(char* filename)
{

}

void SND_StartSoundTrack(void)
{

}

void SND_StopSoundTrack(void)
{

}

int devILinitialized=0;
void loadNativePNG(texture_t* tmpTex)
{
	ILuint texid;
	ILenum error ;

	if (!devILinitialized)
	{
		ilInit(); 
		printf("[DevIL] Initialized.\n");
		devILinitialized = 1;
	}

	ilGenImages(1, &texid); /* Generation of one image name */
	ilBindImage(texid); /* Binding of image name */
	ilLoadImage((const wchar_t*)tmpTex->path);

	tmpTex->bpp = ilGetInteger(IL_IMAGE_BPP);
	tmpTex->width = ilGetInteger(IL_IMAGE_WIDTH) ; 
	tmpTex->height =  ilGetInteger(IL_IMAGE_HEIGHT) ;
	tmpTex->numMipmaps = 1;

	tmpTex->data = (ubyte**)calloc(1,sizeof(ubyte*));
	tmpTex->data[0] = (ubyte*)calloc(tmpTex->width*tmpTex->height*tmpTex->bpp,sizeof(ubyte*));
	tmpTex->dataLength = 0;

	error = ilGetError();

	if (error != IL_NO_ERROR)
	{
		printf("Could not load texture: '%s'\n",tmpTex->path);
		return;
	}


	if (tmpTex->bpp == 4)
	{
		ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE);
		tmpTex->format = TEXTURE_GL_RGBA;
	}
	else
	{
		ilConvertImage(IL_RGB, IL_UNSIGNED_BYTE);
		tmpTex->format = TEXTURE_GL_RGB;
	}

	memcpy(tmpTex->data[0],(void*)ilGetData(), tmpTex->width*tmpTex->height * tmpTex->bpp);
}


void WIN_SwapRenderBuffers()
{
	EGLSwapBuffers();
	FlushWindowsMessages();
}

int main(char** argv, int argc)
{
	uchar engineParameters = 0;
	char cwd[256];
	WORD nBufferLength=256;
	char lpBuffer[256];




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
	renderer.materialQuality = 0;

	gameOn = 1;

	
	dEngine_Init();
	renderer.statsEnabled = 0;

	Create_NativeWindow();

	dEngine_InitDisplaySystem(engineParameters);

	
	

	while(gameOn)
	{
		commands[0].time = simulationTime;
		dEngine_HostFrame();


		WIN_SwapRenderBuffers();
	}


	Destroy_NativeWindow();
	return 0;
}