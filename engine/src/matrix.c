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
 *  matrix.c
 *  dEngine
 *
 *  Created by fabien sanglard on 09/08/09.
 *  Copyright 2009 Memset software Inc. All rights reserved.
 *
 */

#include "matrix.h"
#include "stdio.h"

/*
	Matrices here are colmn major
		
				i
 
		0   4   8  12
		1   5   9  13
j		2   6  10  14
		3   7  11  15
 
	for i=column 
		for j=row,
 
	m[ij] = m[  j + (i*4)  ]
	
*/


void matrixCopy(matrix_t from,matrix_t to)
{
	int i;
	
	for( i = 0 ; i < 16 ; i++)
		to[i] = from[i];
}


void matrix_multiplyVertexByMatrix(vec4_t pos, matrix_t mvp, vec4_t dest)
{

	//Premultiply
	dest[0] = pos[0] * mvp[0] + pos[1] * mvp[4] + pos[2] * mvp[8] + pos[3] * mvp[12];
	dest[1] = pos[0] * mvp[1] + pos[1] * mvp[5] + pos[2] * mvp[9] + pos[3] * mvp[13];
	dest[2] = pos[0] * mvp[2] + pos[1] * mvp[6] + pos[2] * mvp[10] + pos[3] * mvp[14];
	dest[3] = pos[0] * mvp[3] + pos[1] * mvp[7] + pos[2] * mvp[11] + pos[3] * mvp[15];
	
}


void matrix_multiply(const matrix_t m1, const matrix_t m2, matrix_t result)
{
	//Multiply matrix as column major indexation
	
	int i;
	int row=0;
	int colum = 0;
	for( i = 0 ; i < 16 ; i++)
	{
		colum = (i/4) * 4;
		row = i % 4;
		
		result[i] = m1[row] * m2[colum] + m1[row+4] * m2[colum+1] + m1[row+8] * m2[colum+2] + m1[row+12] * m2[colum+3];
	}
	 
}

void matrix_multiply3x3(const matrix3x3_t m1, const matrix3x3_t m2, matrix3x3_t dest)
{
	int i;
	int row=0;
	int colum = 0;
	for( i = 0 ; i < 9 ; i++)
	{
		colum = (i/3) * 3;
		row = i % 3;
		
		dest[i] = m1[row] * m2[colum] + m1[row+3] * m2[colum+1] + m1[row+6] * m2[colum+2] ;
	}
	
}

void matrix_transform_vec4t(const matrix_t m1, const vec4_t vect , vec4_t dest)
{
	//Tried this to enable auto-vectorization on GCC. Generate code with a fucking jump !!!
	/*
	int i;
	for (i=0 ; i < 4; i++) {
		dest[i] = m1[i] * vect[0] +  m1[4+i] * vect[1] + m1[ 8+i] * vect[2] + m1[12+8] * vect[3];
	}
	 */
	
	dest[0] = m1[0] * vect[0] +  m1[4] * vect[1] + m1[ 8] * vect[2] + m1[12] * vect[3];
	dest[1] = m1[1] * vect[0] +  m1[5] * vect[1] + m1[ 9] * vect[2] + m1[13] * vect[3];
	dest[2] = m1[2] * vect[0] +  m1[6] * vect[1] + m1[10] * vect[2] + m1[14] * vect[3];
	dest[3] = m1[3] * vect[0] +  m1[7] * vect[1] + m1[11] * vect[2] + m1[15] * vect[3];	
	 
}

void matrix_transform_vec3t(const matrix3x3_t m1, const vec3_t vect , vec3_t dest)
{
	dest[0] = m1[0] * vect[0] +  m1[3] * vect[1] + m1[6] * vect[2] ;
	dest[1] = m1[1] * vect[0] +  m1[4] * vect[1] + m1[7] * vect[2] ;
	dest[2] = m1[2] * vect[0] +  m1[5] * vect[1] + m1[8] * vect[2] ;	
}

void matrix_print(matrix_t m)
{
	int i,j;
	
	printf("-----------------\n");
	for( i = 0 ; i < 4 ; i++)		//Column dest
	{
		for( j = 0 ; j < 4 ; j++)	//Row dest
		{
			// m1 j row * m2 i column
			printf(" %.4f ", m[i+j*4]);
		}
		printf("\n");
	}
	printf("-----------------\n");
}


void matrix_print3x3(matrix3x3_t m)
{
	int i,j;
	
	printf("-----------------\n");
	for( i = 0 ; i < 3 ; i++)		//Column dest
	{
		for( j = 0 ; j < 3 ; j++)	//Row dest
		{
			// m1 j row * m2 i column
			printf(" %.4f ", m[i+j*3]);
		}
		printf("\n");
	}
	printf("-----------------\n");
}

void matrixLoadIdentity(matrix_t m)
{
	int i;
	for (i=0 ; i < 16; i++) 
		m[i] = 0;
	
	m[0] = 1;
	m[5] = 1;
	m[10] = 1;
	m[15] = 1;	
}