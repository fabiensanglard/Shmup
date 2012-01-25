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
 *  quaternion.c
 *  dEngine
 *
 *  Created by fabien sanglard on 10/08/09.
 *  Copyright 2009 Memset software Inc. All rights reserved.
 *
 */

#include "quaternion.h"


/**
 * Basic quaternion operations.
 */

void Quat_computeW (quat4_t q)
{
	float t = 1.0f - (q[X] * q[X]) - (q[Y] * q[Y]) - (q[Z] * q[Z]);
	
	if (t < 0.0f)
		q[W] = 0.0f;
	else
		q[W] = -sqrt (t);
}

void Quat_normalize (quat4_t q)
{
	/* compute magnitude of the quaternion */
	float mag = sqrt ((q[X] * q[X]) + (q[Y] * q[Y])
					  + (q[Z] * q[Z]) + (q[W] * q[W]));
	
	/* check for bogus length, to protect against divide by zero */
	if (mag > 0.0f)
    {
		/* normalize it */
		float oneOverMag = 1.0f / mag;
		
		q[X] *= oneOverMag;
		q[Y] *= oneOverMag;
		q[Z] *= oneOverMag;
		q[W] *= oneOverMag;
    }
}

void Quat_multQuat (const quat4_t qa, const quat4_t qb, quat4_t out)
{
	out[W] = (qa[W] * qb[W]) - (qa[X] * qb[X]) - (qa[Y] * qb[Y]) - (qa[Z] * qb[Z]);
	out[X] = (qa[X] * qb[W]) + (qa[W] * qb[X]) + (qa[Y] * qb[Z]) - (qa[Z] * qb[Y]);
	out[Y] = (qa[Y] * qb[W]) + (qa[W] * qb[Y]) + (qa[Z] * qb[X]) - (qa[X] * qb[Z]);
	out[Z] = (qa[Z] * qb[W]) + (qa[W] * qb[Z]) + (qa[X] * qb[Y]) - (qa[Y] * qb[X]);
}

void Quat_multVec (const quat4_t q, const vec3_t v, quat4_t out)
{
	out[W] = - (q[X] * v[X]) - (q[Y] * v[Y]) - (q[Z] * v[Z]);
	out[X] =   (q[W] * v[X]) + (q[Y] * v[Z]) - (q[Z] * v[Y]);
	out[Y] =   (q[W] * v[Y]) + (q[Z] * v[X]) - (q[X] * v[Z]);
	out[Z] =   (q[W] * v[Z]) + (q[X] * v[Y]) - (q[Y] * v[X]);
}

void Quat_rotateShortPoint (const quat4_t q, const vec3short_t in, vec3_t out)
{
	vec3_t inFloat;
	
	inFloat[0] = in[0];
	inFloat[1] = in[1];
	inFloat[2] = in[2];
	
	Quat_rotatePoint(q,inFloat,out);
	
	
}

void Quat_rotatePoint (const quat4_t q, const vec3_t in, vec3_t out)
{
	quat4_t tmp, inv, final;
	
	inv[X] = -q[X]; 
	inv[Y] = -q[Y];
	inv[Z] = -q[Z]; 
	inv[W] =  q[W];
	
	//Quat_normalize (inv);
	
	Quat_multVec (q, in, tmp);
	Quat_multQuat (tmp, inv, final);
	
	out[X] = final[X];
	out[Y] = final[Y];
	out[Z] = final[Z];
}

void multiplyByInvertQuaternion( const vec3_t v1, const quat4_t quat, vec3_t dest)
{
	static quat4_t inv;
	
	inv[0] = -quat[0]; 
	inv[1] = -quat[1];
	inv[2] = -quat[2]; 
	inv[3] =  quat[3];
	
	//printf("joint inverse : x=%.3f,y=%.3f,z=%.3f  w=%.3f \n",inv[0],inv[1],inv[2],inv[3]);				
	
	Quat_rotatePoint (inv,v1,dest);
	
}

float ReciprocalSqrt( float x ) 
{
	long i;
	float y, r;
	y = x * 0.5f; 
	i = *(long *)( &x ); 
	i = 0x5f3759df - ( i >> 1 ); 
	r = *(float *)( &i ); 
	r = r * ( 1.5f - r * r * y );
	return r;
}

void Quat_CreateFromMat3x3(const matrix3x3_t m,quat4_t q)
{
	float s;
	float t;
	//   0 1  2  3      0 3 6
	//   4 5  6  7  ->  1 4 7 
	//   8 9 10 11      2 5 8
	
	if ( m[0] + m[4] + m[8] > 0)
	{
		t = + m[0] + m[4] + m[8] + 1;
		s = ReciprocalSqrt(t) * 0.5f;
		q[3] = s * t;
		q[2] = ( m[3] - m[1] ) * s;
		q[1] = ( m[2] - m[6] ) * s;
		q[0] = ( m[7] - m[5] ) * s;
	}
	else if ( m[0] > m[4] && m[0] > m[8] ) 
	{	// Column 0: 
		t = + m[0] - m[4] - m[8] + 1;
		s = ReciprocalSqrt(t) * 0.5f;

		q[0] = s * t;
		q[1] = (m[3] + m[1] ) * s;
		q[2] = (m[6] + m[2] ) * s;
		q[3] = (m[7] - m[5] ) * s;
	} 
	else if ( m[4] > m[8] ) 
	{	// Column 1: 
		t = - m[0] + m[4] - m[8] + 1;
		s = ReciprocalSqrt(t) * 0.5f;
		
		q[1] =  s * t;
		q[0] = (m[3] + m[1] ) * s;
		q[3] = (m[2] - m[6] ) * s;
		q[2] = (m[7] + m[5] ) * s;
		
	} 
	else 
	{	// Column 2:
		t = - m[0] - m[4] + m[8] + 1;
		s = ReciprocalSqrt(t) * 0.5f;

		q[2] =  s * t;
		q[3] = (m[3] - m[1] ) * s;
		q[0] = (m[2] + m[6] ) * s;
		q[1] = (m[7] + m[5] ) * s;
		
	}
	
	Quat_normalize(q);
	
}

void Quat_ConvertToMat3x3(matrix3x3_t m,const quat4_t q)
{
	/*
	matrix[0] = 1-2* (out[Y]*out[Y] + out[Z]*out[Z]);	matrix[1] =   2* (out[X]*out[Y] - out[W]*out[Z]);	matrix[2] =   2* (out[X]*out[Z] + out[W]*out[Y]);
	matrix[3] =   2* (out[X]*out[Y] + out[W]*out[Z]);	matrix[4] = 1-2* (out[X]*out[X] + out[Z]*out[Z]);	matrix[5] =   2* (out[Y]*out[Z] - out[W]*out[X]);
	matrix[6] =   2* (out[X]*out[Z] - out[W]*out[Y]);	matrix[7] =   2* (out[Y]*out[Z] + out[W]*out[X]);	matrix[8] = 1-2* (out[X]*out[X] + out[Y]*out[Y]);	
	 */
	
	/*
	matrix[0] = 1-2* (out[Y]*out[Y] + out[Z]*out[Z]);	matrix[3] =   2* (out[X]*out[Y] - out[W]*out[Z]);	matrix[6] =   2* (out[X]*out[Z] + out[W]*out[Y]);
	matrix[1] =   2* (out[X]*out[Y] + out[W]*out[Z]);	matrix[4] = 1-2* (out[X]*out[X] + out[Z]*out[Z]);	matrix[7] =   2* (out[Y]*out[Z] - out[W]*out[X]);
	matrix[2] =   2* (out[X]*out[Z] - out[W]*out[Y]);	matrix[5] =   2* (out[Y]*out[Z] + out[W]*out[X]);	matrix[8] = 1-2* (out[X]*out[X] + out[Y]*out[Y]);	
	*/
	
	// Following code is from idSoftware's J.M.P. van Waveren
	// modified to populate a column major matrix with no translation component
	/*
	 // We are not storing a translation in 3x3 matrices, Doom3 was
	m[0*4+3] = q[4]; 
	m[1*4+3] = q[5]; 
	m[2*4+3] = q[6];
	*/
	
	//   0 1  2  3      0 3 6
	//   4 5  6  7  ->  1 4 7 
	//   8 9 10 11      2 5 8
	
	float x2 = q[0] + q[0]; 
	float y2 = q[1] + q[1]; 
	float z2 = q[2] + q[2]; 
	{
		float xx2 = q[0] * x2; 
		float yy2 = q[1] * y2; 
		float zz2 = q[2] * z2;
	
		/*m[0*4+0]*/ m[0] = 1.0f - yy2 - zz2; 
		/*m[1*4+1]*/ m[4] = 1.0f - xx2 - zz2; 
		/*m[2*4+2]*/ m[8] = 1.0f - xx2 - yy2;
	}
	

	{
		float yz2 = q[1] * z2; 
		float wx2 = q[3] * x2;
		
		/*m[2*4+1]*/ m[5] = yz2 - wx2; 
		/*m[1*4+2]*/ m[7] = yz2 + wx2;
	} 
	
	{
		float xy2 = q[0] * y2; 
		float wz2 = q[3] * z2;
		/*m[1*4+0]*/ m[1] = xy2 - wz2; 
		/*m[0*4+1]*/ m[3] = xy2 + wz2;
	} 
	
	{
		float xz2 = q[0] * z2; 
		float wy2 = q[3] * y2;
		
		/*m[0*4+2]*/ m[6] = xz2 - wy2; 
		/*m[2*4+0]*/ m[2] = xz2 + wy2;

	}
		
	
}


void Quat_slerp (const quat4_t qa, const quat4_t qb, float t, quat4_t out)
{
	float cosOmega,sinOmega,omega,oneOverSinOmega ;
	float q1w,q1x,q1y,q1z; 
	float k0, k1;

	/* Check for out-of range parameter and return edge points if so */
	if (t <= 0.0)
    {
		memcpy (out, qa, sizeof(quat4_t));
		return;
    }
	
	if (t >= 1.0)
    {
		memcpy (out, qb, sizeof (quat4_t));
		return;
    }
	
	/* Compute "cosine of angle between quaternions" using dot product */
	 cosOmega = Quat_dotProduct (qa, qb);
	
	/* If negative dot, use -q1.  Two quaternions q and -q
     represent the same rotation, but may produce
     different slerp.  We chose q or -q to rotate using
     the acute angle. */
	q1w = qb[W];
	q1x = qb[X];
	q1y = qb[Y];
	q1z = qb[Z];
	
	if (cosOmega < 0.0f)
    {
		q1w = -q1w;
		q1x = -q1x;
		q1y = -q1y;
		q1z = -q1z;
		cosOmega = -cosOmega;
    }	
	
	/* Compute interpolation fraction, checking for quaternions
     almost exactly the same */
	
	
	if (cosOmega > 0.9999f)
    {
		/* Very close - just use linear interpolation,
		 which will protect againt a divide by zero */
		
		k0 = 1.0f - t;
		k1 = t;
    }
	else
    {
		/* Compute the sin of the angle using the
		 trig identity sin^2(omega) + cos^2(omega) = 1 */
		sinOmega = sqrt (1.0f - (cosOmega * cosOmega));
		
		/* Compute the angle from its sin and cosine */
		omega = atan2 (sinOmega, cosOmega);
		
		/* Compute inverse of denominator, so we only have
		 to divide once */
		oneOverSinOmega = 1.0f / sinOmega;
		
		/* Compute interpolation parameters */
		k0 = sin ((1.0f - t) * omega) * oneOverSinOmega;
		k1 = sin (t * omega) * oneOverSinOmega;
    }
	
	/* Interpolate and return new quaternion */
	out[W] = (k0 * qa[3]) + (k1 * q1w);
	out[X] = (k0 * qa[0]) + (k1 * q1x);
	out[Y] = (k0 * qa[1]) + (k1 * q1y);
	out[Z] = (k0 * qa[2]) + (k1 * q1z);
}

float Quat_dotProduct (const quat4_t qa, const quat4_t qb)
{
	return ((qa[X] * qb[X]) + (qa[Y] * qb[Y]) + (qa[Z] * qb[Z]) + (qa[W] * qb[W]));
}