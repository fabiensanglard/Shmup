#include "../src/native_services.h"
#include "../src/texture.h"






#include <IL/il.h>
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

	ilGenImages(1, &texid); // Generation of one image name 
	ilBindImage(texid); // Binding of image name 
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



//On windows there is no gamecenter, those method are here just to satisfy the linker.


int  Native_RetrieveListOf(char replayListIn[10][256])
{

	return 0;
}
void Native_UploadFileTo(char path[256]){}
void Action_ShowGameCenter(void* tag){}
void Native_UploadScore(uint score){}
void Native_LoginGameCenter(void){}



void SND_InitSoundTrack(char* filename)
{

}

void SND_StartSoundTrack(void)
{

}

void SND_StopSoundTrack(void)
{

}