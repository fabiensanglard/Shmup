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
 *  math.h
 *  dEngine
 *
 *  Created by fabien sanglard on 10/08/09.
 *  Copyright 2009 Memset software Inc. All rights reserved.
 *
 */

#include "config.h"

#ifndef DE_MATH
#define DE_MATH 





/* Vectors */
typedef float vec2_t[2];
typedef float vec3_t[3];
typedef float vec4_t[4];
typedef short vec4short_t[4];
typedef short vec3short_t[3];
typedef short vec2short_t[2];


typedef  struct vertex_t {
	vec3_t pos;
	vec3short_t normal;
	vec2short_t text;
	vec3short_t tangent;
} vertex_t;
#define VERTEX_T_DELTA_TO_POS 0
#define VERTEX_T_DELTA_TO_NORMAL 12
#define VERTEX_T_DELTA_TO_TEXT 18
#define VERTEX_T_DELTA_TO_TANGEN 24

/*
typedef  struct svertex_t {
	vec3short_t pos;
	vec2_t text;
} svertex_t;
*/


#define vectorClear( a )		( (a)[ 0 ] = (a)[ 1 ] = (a)[ 2 ] = 0 )
#define vectorNegate( a, b )	( (b)[ 0 ] = -(a)[ 0 ], (b)[ 1 ] = -(a)[ 1 ], (b)[ 2 ] = -(a)[ 2 ] )
#define vectorSet( v, x, y, z )	( (v)[ 0 ] = ( x ), (v)[ 1 ] = ( y ), (v)[ 2 ] = ( z ) )
#define vectorInverse( a )		( (a)[ 0 ] = (-a)[ 0 ], (a)[ 1 ] = (-a)[ 1 ], (a)[ 2 ] = (-a)[ 2 ] )
#define DotProduct( x, y )			( (x)[ 0 ] * (y)[ 0 ] + (x)[ 1 ] * (y)[ 1 ] + (x)[ 2 ] * (y)[ 2 ] )
#define vectorSubtract( a, b, c )	( (c)[ 0 ] = (a)[ 0 ] - (b)[ 0 ], (c)[ 1 ] = (a)[ 1 ] - (b)[ 1 ], (c)[ 2 ] = (a)[ 2 ] - (b)[ 2 ] )
#define vectorAdd( a, b, c )		( (c)[ 0 ] = (a)[ 0 ] + (b)[ 0 ], (c)[ 1 ] = (a)[ 1 ] + (b)[ 1 ], (c)[ 2 ] = (a)[ 2 ] + (b)[ 2 ] )
#define vectorCopy( a, b )			( (b)[ 0 ] = (a)[ 0 ], (b)[ 1 ] = (a)[ 1 ], (b)[ 2 ] = (a)[ 2 ] )
#define	vectorScale( v, s, o )		( (o)[ 0 ] = (v)[ 0 ] * (s),(o)[ 1 ] = (v)[ 1 ] * (s), (o)[ 2 ] = (v)[ 2 ] * (s) )
#define	vectorMA( v, s, b, o )		( (o)[ 0 ] = (v)[ 0 ] + (b)[ 0 ]*(s),(o)[ 1 ] = (v)[ 1 ] + (b)[ 1 ] * (s),(o)[ 2 ] = (v)[ 2 ] + (b)[ 2 ] * (s) )
#define vector_Interpolate( vtarget, v1, value, v2 ) ( (vtarget)[0] = (v1)[0] * (1 - (value)) + (v2)[0] * (value),   (vtarget)[1] = (v1)[1] * (1 - (value)) + (v2)[1] * (value)  , (vtarget)[2] = (v1)[2] * (1 - (value)) + (v2)[2] * (value)  )

#define vector4Copy( a, b )			( (b)[ 0 ] = (a)[ 0 ], (b)[ 1 ] = (a)[ 1 ], (b)[ 2 ] = (a)[ 2 ], (b)[ 3 ] = (a)[ 3 ] )

#define vector2Clear( a )		( (a)[ 0 ] = (a)[ 1 ] = 0 )
#define vector2Subtract( a, b, c )	( (c)[ 0 ] = (a)[ 0 ] - (b)[ 0 ], (c)[ 1 ] = (a)[ 1 ] - (b)[ 1 ])
#define vector2Copy( a, b )			( (b)[ 0 ] = (a)[ 0 ], (b)[ 1 ] = (a)[ 1 ] )
#define	vector2Scale( v, s, o )		( (o)[ 0 ] = (v)[ 0 ] * (s),(o)[ 1 ] = (v)[ 1 ] * (s) )

void vectorCrossProduct( const vec3_t v1, const vec3_t v2, vec3_t cross );
void normalize(vec3_t v);
void normalize2(vec2_t v);
void vectorLinearInterpolate(const vec3_t v1,const vec3_t v2,float t,vec3_t dest);
void vectorCosinuInterpolate(const vec3_t v1,const vec3_t v2,float t,vec3_t dest);

#ifndef MAX
#define MAX(A,B) ((A) > (B))? (A) : (B)
#endif

#ifndef MIN
#define MIN(A,B) ((A) < (B))? (A) : (B)
#endif

void strReplace(char* string,char toReplace, char replacment);

#endif