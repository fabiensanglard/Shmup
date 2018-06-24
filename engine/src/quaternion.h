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
 *  quaternion.h
 *  dEngine
 *
 *  Created by fabien sanglard on 10/08/09.
 *  Copyright 2009 Memset software Inc. All rights reserved.
 *
 */



#ifndef DE_QUATERNION
#define DE_QUATERNION

#include "globals.h"
#include "math.h"
#include "matrix.h"


/* Quaternion (x, y, z, w) */
typedef float quat4_t[4];


void Quat_computeW (quat4_t q);
void Quat_normalize (quat4_t q);
void Quat_multQuat (const quat4_t qa, const quat4_t qb, quat4_t out);
void Quat_multVec (const quat4_t q, const vec3_t v, quat4_t out);
void multiplyByInvertQuaternion( const vec3_t v1, const quat4_t quat, vec3_t dest);
void Quat_rotatePoint (const quat4_t q, const vec3_t in, vec3_t out);
void Quat_rotateShortPoint (const quat4_t q, const vec3short_t in, vec3_t out);

float Quat_dotProduct (const quat4_t qa, const quat4_t qb);
void Quat_slerp (const quat4_t qa, const quat4_t qb, float t, quat4_t out);
void Quat_CreateFromMat3x3(const matrix3x3_t matrix,quat4_t out);
void Quat_ConvertToMat3x3(matrix3x3_t matrix,const quat4_t out);
#endif