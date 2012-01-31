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
/*
 *  globals.h
 *  dEngine
 *
 *  Created by fabien sanglard on 09/08/09.
 *  Copyright 2009 Memset software Inc. All rights reserved.
 *
 */

#ifndef ED_GLOBAL
#define ED_GLOBAL


typedef unsigned int uint ;
typedef unsigned char uchar ;
typedef unsigned char ubyte ;
typedef unsigned short ushort ;



#include <stdio.h>
//Doing some memory tracking here
//#include <stdlib.h>
#include "trackmem.h"
#include <string.h>
#include <math.h>
#include <limits.h>

#ifndef M_PI
#define M_PI 3.14159265f
#endif

#define DEG_TO_RAD (2.0f*M_PI/360.0f)
enum {	X, Y, Z, W };
enum {	U,  V };
enum {R, G, B , A}  ;
enum {WIDTH, HEIGHT}  ;
#define DE_USHRT_MAX 65535
#define DE_SHRT_MAX 32767

/*
  A few words about the coordinate systems used by Shmup:

  Since the game must run on a lot of different hardwares with different screen size we have the concept of screen active surface and screen inactive surface.
  On an iPhone with 320/480 screen , the entire screen is considered active surface.
  On an iPad with 780/1024 screen , the active surface is actually 682x1024. We have two black bands 784-682 = 102: 51 pixels wide each (and or course 1024 high).
  The two black bandes are inactive surface.

  The openGL viewPort is set to draw the active surface centered on the screen. 
       - In the case of the iPad the viewport is rendering at an offset (51,0) and dimension w=682 h-1024.
	   - In the case of the iPhone the viewport is set to render at 0,0 with w=320 and h=1024.











  Menus and buttons are placed in a coordinate system where 0,0 is at the center and the width is 320 and height 480 (so coordinate limits are [-160,160] and [-240,240])
    
  In order to make inputs interaction easier to interprect a button also features an "iphone_coo_SysPos" where coordinate system is centered at the upper left and is [0,320] [0,480].
  This is the SHMUP virtual screen (it actually matches the original iphone screen coordinate system).


*/


//#define GENERATE_VIDEO

//Here are the Shmup active surface legacy dimensions.
#define SS_COO_SYST_WIDTH  320
#define SS_COO_SYST_HEIGHT 480

#define SS_W SS_COO_SYST_WIDTH
#define SS_H SS_COO_SYST_HEIGHT

#define NUM_BUTTONS  3

#ifdef WIN32
  #pragma warning(disable: 4244)
 #pragma warning(disable: 4305)

#endif

#endif