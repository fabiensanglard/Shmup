#include "../../../src/filesystem.h"
#include "android/asset_manager.h"
#include "android/native_activity.h"

#define  LOG_TAG    		"net.fabiensanglard.native"
#include <android/log.h>
#define printf(fmt,args...) __android_log_print(ANDROID_LOG_INFO  ,LOG_TAG, fmt, ##args)

//For struct android_app
#include <android_native_app_glue.h>




//We access all the assets in the APK via the asset manager.
AAssetManager*      assetManager;

void FS_AndroidPreInitFileSystem(struct android_app* state){

	ANativeActivity*    activity  ;

	printf("[FS_AndroidPreInitFileSystem]\n");

	activity = state->activity;
	assetManager = activity->assetManager;

	/*
	//Make sure the directory where we will be writing (logs, replays) exists.
	const char* assetURL;
	AAssetDir* directory = AAssetManager_openDir(assetManager,"assets");

	while(assetURL = AAssetDir_getNextFileName(directory))
		printf(assetURL);

	fflush(stdout);

	AAssetDir_close(directory);
	*/
}

void	FS_InitFilesystem(void){


}

char*	FS_Gamedir(void){
	return NULL;
}

filehandle_t* FS_OpenFile( const char *filename, char* mode  ){

	char androidFilename[256];
	AAsset* asset;
	filehandle_t* file;

	strcpy(androidFilename,"assets/");
	strcat(androidFilename,filename);

	asset = AAssetManager_open(assetManager, filename,AASSET_MODE_BUFFER);

	file = calloc(1,sizeof(filehandle_t));
	file->hFile = asset;
	file->filesize = AAsset_getLength(asset);

	return file;
}

int FS_UploadToRAM(filehandle_t *fhandle){

	AAsset* asset = fhandle->hFile;

	fhandle->filedata = calloc(1,fhandle->filesize);

	AAsset_read(asset, fhandle->filedata,fhandle->filesize);

	fhandle->ptrStart =  fhandle->ptrCurrent = (PW8)fhandle->filedata;
	fhandle->ptrEnd =  (PW8)fhandle->filedata + fhandle->filesize;
	fhandle->bLoaded = 1;
	fhandle->isWritable = 0;
}

void FS_CloseFile( filehandle_t *fhandle ){
	//If the file was uploaded to RAM we need to free the buffer.
		if( fhandle->filedata )
		{

			free( fhandle->filedata );
			fhandle->filedata = NULL;
		}

		AAsset_close( fhandle->hFile);

		free( fhandle );

}

SW32 FS_Read(         void *buffer, W32 size, W32 count, filehandle_t * fhandle ){
	AAsset* asset = fhandle->hFile;
	return  AAsset_read(asset,buffer,size*count);
}

SW32 FS_Write( const void * buffer, W32 size, W32 count, filehandle_t * fhandle ){
	return 0;
}

void *FS_GetLoadedFilePointer( filehandle_t *fhandle, W32 origin )
{
	switch( origin )
	{
		case SEEK_SET:
			return( (void *)fhandle->ptrStart );

		case SEEK_END:
			return( (void *)fhandle->ptrEnd );

		case SEEK_CUR:
			return( (void *)fhandle->ptrCurrent );
	}

	return NULL;
}


char* FS_GetExtensionAddress(char* string)
{
	char* extension;

	extension = &string[strlen(string)-1];

	while(*extension != '.' && extension != string)
		extension--;

	return (extension+1);
}

char* FS_GetFilenameOnly(char* string)
{
	char* filename = &string[strlen(string)-1];;

	while (filename != string) {
		if (*filename == '\\' || *filename == '/')
			return filename+1;
		filename--;
	}

	return filename;
}
