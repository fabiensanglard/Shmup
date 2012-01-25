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


//#define GENERATE_VIDEO


#define SS_COO_SYST_WIDTH  320
#define SS_COO_SYST_HEIGHT 480

#define SS_W SS_COO_SYST_WIDTH
#define SS_H SS_COO_SYST_HEIGHT

#define NUM_BUTTONS  3

#endif