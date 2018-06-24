/*
SHMUP is a 3D Shoot 'em up game inspired by Treasure Ikaruga
Copyright (C) 2009 Fabien Sanglard

This file is part of SCHMUP.

SCHMUP is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

SCHMUP is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with SCHMUP.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "SDL_mixer.h"
#include "libpng/png.h"

#include "globals.h"
#include "music.h"
#include "native_services.h"
#include "texture.h"

int  Native_RetrieveListOf(char replayList[10][256]) { return 0; }
void Native_UploadFileTo(char path[256]) {}
void Action_ShowGameCenter(void* tag) {}
void Native_UploadScore(uint score) {}
void Native_LoginGameCenter(void) {}

Mix_Music *music = NULL;
void SND_InitSoundTrack(char* filename, unsigned int startAt)
{
    int audio_rate = 22050;
    int audio_channels = 2;
    Uint16 audio_format = AUDIO_S16;

    (void)startAt; /* Unused */

    if (music)
        Mix_FreeMusic(music);

    Log_Printf("[SND_InitSoundTrack] start '%s'.\n", filename);

    Mix_OpenAudio(audio_rate, audio_format, audio_channels, 4096);
    Mix_QuerySpec(&audio_rate, &audio_format, &audio_channels);

    music = Mix_LoadMUS(filename);
}

void SND_StartSoundTrack(void)
{
    if (music == NULL)
        return;

    Mix_PlayMusic(music, 0);
}

void SND_StopSoundTrack(void)
{
    if (music == NULL)
        return;

    Mix_HaltMusic();
}

void loadNativePNG(texture_t* tmpTex)
{
    png_structp     png_ptr; 
    png_infop       info_ptr; 
    unsigned int    width;
    unsigned int    height;
    int             i;
    
    int             bit_depth;
    int             color_type ;
    png_size_t      rowbytes;
    png_bytep       *row_pointers;
    char* file_name = tmpTex->path;
    uchar header[8];
    
    int             number_of_passes;
    int             interlace_type;
    
    char realPath[1024];
    FILE *fp = NULL ;
    
    memset(realPath,0,1024);    
    strcat(realPath,FS_Gamedir());
    strcat(realPath,"/");
    strcat(realPath,tmpTex->path);
    
    tmpTex->format = TEXTURE_TYPE_UNKNOWN ;
    
    fp = fopen(realPath,"rb");
    
    if ( !fp  )
        return;
    
    
    // Check signature (it should be Hex: 89 50 4E 47 0D 0A 1A 0A)
    //                               Dec:137 80 78 71 13 10 26 10
    fread(header, 1, 8, fp);
    if (png_sig_cmp(header, 0, 8) != 0 )
    {
        printf("[read_png_file] File '%s' is not recognized as a PNG file.\n", file_name);
        return;
    }
    
    
    // initialize
    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    
    if (png_ptr == NULL){
        printf("[read_png_file] png_create_read_struct failed");
        return;
    }
    
    info_ptr = png_create_info_struct(png_ptr);
    if (info_ptr == NULL){
        printf("[read_png_file] png_create_info_struct failed");
        return;
    }
    
    // FCS: Shoud NOT CRASH AROUND HERE 
    if (setjmp(png_jmpbuf(png_ptr))){
        printf("[read_png_file] Error during init_io");
        return;
    }
    
    // FCS: By the way, that is probably where it was crashing: On first read.
    png_init_io(png_ptr, fp);
    png_set_sig_bytes(png_ptr, 8);
    
    png_read_info(png_ptr, info_ptr);
    
    //Retrieve metadata and tranfert to structure bean tmpTex
    png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, &interlace_type, NULL, NULL);
    
    
    if (interlace_type == PNG_INTERLACE_ADAM7)
        number_of_passes= png_set_interlace_handling(png_ptr);
    
    
    tmpTex->width = width; 
    tmpTex->height =  height;
    

    
    
    
    /* Set up some transforms. */
    //FCS: WTF Why are we trowing away the alpha channel ?! This is wrong....
//    if (color_type & PNG_COLOR_MASK_ALPHA) {
//      png_set_strip_alpha(png_ptr);
//    }
    
    if (bit_depth > 8) {
        png_set_strip_16(png_ptr);
    }
    if (color_type == PNG_COLOR_TYPE_GRAY ||
        color_type == PNG_COLOR_TYPE_GRAY_ALPHA) {
        png_set_gray_to_rgb(png_ptr);
    }
    if (color_type == PNG_COLOR_TYPE_PALETTE) {
        png_set_palette_to_rgb(png_ptr);
    }
    
    /* Update the png info struct.*/
    png_read_update_info(png_ptr, info_ptr);
    
    /* Rowsize in bytes. */
    rowbytes = png_get_rowbytes(png_ptr, info_ptr);
    
    tmpTex->bpp = rowbytes / width;
    if (tmpTex->bpp == 4)
        tmpTex->format = TEXTURE_GL_RGBA;
    else if (tmpTex->bpp == 3)
        tmpTex->format = TEXTURE_GL_RGB;
    else
    {
        printf("!!! ERROR !! PNG %s is not a supported format.",tmpTex->path);
    }
    
    
    /* Allocate a buffer to hold all the mip-maps */
    //Since PNG can only store one image there is only one mipmap, allocated an array of one
    tmpTex->numMipmaps = 1;
    tmpTex->data = malloc(sizeof(uchar*));
    if ((tmpTex->data[0] = (uchar*)malloc(rowbytes * height))==NULL) 
    {
        //Oops texture won't be able to hold the result :(, cleanup LIBPNG internal state and return;
        free(tmpTex->data);
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        return;
    }
    
    //Next we need to send to libpng an array of pointer, let's point to tmpTex->data[0]
    if ((row_pointers = (png_bytepp)malloc(height*sizeof(png_bytep))) == NULL) 
    {
        // Oops looks like we won't have enough RAM to allocate an array of pointer (are 
        // you running this on a Motorola Razor ?!? 
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        free(tmpTex->data );
        tmpTex->data  = NULL;
        return;
    }
    //FCS: Hm, it looks like we are flipping the image vertically.
    //     Since iOS did not do it, we may have to not to that. If result is 
    //     messed up, just swap to:   row_pointers[             i] = ....
    for (i = 0;  i < height;  ++i)
    //    row_pointers[height - 1 - i] = tmpTex->data[0]  + i*rowbytes;
          row_pointers[             i] = tmpTex->data[0]  + i*rowbytes;
    
    
    
    
    //Decompressing PNG to RAW where row_pointers are pointing (tmpTex->data[0])
    png_read_image(png_ptr, row_pointers);
    
    //Last but not least:
    
    //Free the decompression buffer
    free(row_pointers);
    
    // Free LIBPNG internal state.
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    
    fclose(fp);
}
