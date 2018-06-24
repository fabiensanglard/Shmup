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
    along with SHMUP.  If not, see <http://www.gnu.org/licenses/>.
*/    
/*
 *  math.c
 *  dEngine
 *
 *  Created by fabien sanglard on 10/08/09.
 *  Copyright 2009 Memset software Inc. All rights reserved.
 *
 */

#include "globals.h"
#include "math.h"
#include <math.h>
#include "string.h"

void vectorCrossProduct( const vec3_t v1, const vec3_t v2, vec3_t cross ) 
{
	cross[ 0 ] = v1[ 1 ] * v2[ 2 ] - v1[ 2 ] * v2[ 1 ];		// X
	cross[ 1 ] = v1[ 2 ] * v2[ 0 ] - v1[ 0 ] * v2[ 2 ];		// Y
	cross[ 2 ] = v1[ 0 ] * v2[ 1 ] - v1[ 1 ] * v2[ 0 ];		// Z
}

// Long life to however came up with this. You rule man.
float InvSqrt(float x)
{
	float xhalf = 0.5f*x; 
	int i = *(int*)&x;	// get bits for floating value 
	i = 0x5f3759df - (i>>1); // gives initial guess y0 
	x = *(float*)&i;	// convert bits back to float 
	x = x*(1.5f-xhalf*x*x); // Newton step, repeating increases accuracy 
	return x; 
}

void normalize(vec3_t v)
{
	float ilength;
	//float length;
	//length = (float)sqrt( v[ 0 ] * v[ 0 ] + v[ 1 ] * v[ 1 ] + v[ 2 ] * v[ 2 ] );
	
	ilength = InvSqrt(v[ 0 ] * v[ 0 ] + v[ 1 ] * v[ 1 ] + v[ 2 ] * v[ 2 ]);
//	printf("Length = %.3f\n",length);
	
	//if( length )
	//{
		//ilength = 1 / length;
		v[ 0 ] *= ilength;
		v[ 1 ] *= ilength;
		v[ 2 ] *= ilength;
	//}
}

void normalize2(vec2_t v)
{
	float ilength;
	//float length;
	//length = (float)sqrt( v[ 0 ] * v[ 0 ] + v[ 1 ] * v[ 1 ] );
	ilength = InvSqrt(v[ 0 ] * v[ 0 ] + v[ 1 ] * v[ 1 ]);
	//	printf("Length = %.3f\n",length);
	
	//if( length )
//	{
		//ilength = 1 / length;
		v[ 0 ] *= ilength;
		v[ 1 ] *= ilength;
//	}
}

void vectorLinearInterpolate(const vec3_t v1,const vec3_t v2,float f,vec3_t dest)
{
	dest[0] = (1-f) *v1[0] + f * v2[0] ;
	dest[1] = (1-f) *v1[1] + f * v2[1] ;
	dest[2] = (1-f) *v1[2] + f * v2[2] ;
}

void vectorCosinuInterpolate(const vec3_t v1,const vec3_t v2,float f,vec3_t dest)
{
	f = (1-cosf(f*M_PI))/2;
	
	//return(y1*(1-mu2)+y2*mu2);
	
	dest[0] = (1-f) *v1[0] + f * v2[0] ;
	dest[1] = (1-f) *v1[1] + f * v2[1] ;
	dest[2] = (1-f) *v1[2] + f * v2[2] ;
}

void strReplace(char* string,char toReplace, char replacment)
{
	size_t i;
	
	for (i=0; i < strlen(string); i++) {
		if (string[i] == toReplace)
			string[i] = replacment;
	}
}
