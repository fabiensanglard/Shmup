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
 *  matrix.h
 *  dEngine
 *
 *  Created by fabien sanglard on 09/08/09.
 *  Copyright 2009 Memset software Inc. All rights reserved.
 *
 */


#ifndef DE_MATRIX
#define DE_MATRIX

#include "math.h"

typedef float matrix_t[16];
void matrix_multiplyVertexByMatrix(vec3_t pos, matrix_t modelViewProjectionMatrix, vec3_t dest);
void matrix_multiply(const matrix_t m1, const matrix_t m2, matrix_t dest);
void matrix_transform_vec4t(const matrix_t m1, const vec4_t vect , vec4_t dest);
void matrix_print(matrix_t m);
void matrixLoadIdentity(matrix_t m);
void matrixCopy(matrix_t from,matrix_t to);



typedef float matrix3x3_t[9];
void matrix_multiply3x3(const matrix3x3_t m1, const matrix3x3_t m2, matrix3x3_t dest);
void matrix_print3x3(matrix3x3_t m);
void matrix_transform_vec3t(const matrix3x3_t m1, const vec3_t vect , vec3_t dest);

#endif