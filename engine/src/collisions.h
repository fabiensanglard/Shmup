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
 *  collisions.h
 *  dEngine
 *
 *  Created by fabien sanglard on 09-11-15.
 *  Copyright 2009 Memset software Inc. All rights reserved.
 *
 */

#ifndef DE_COLLISIONS
#define DE_COLLISIONS

#include "math.h"
#include "globals.h"
#include "matrix.h"
#include "math.h"
#include "preproc.h"


typedef enum { INT_IN,INT_OUT,INT_INTERSEC } Intersection_test;

//Mostly used during pre-processing
typedef vec3_t bbox_t[8];

typedef struct plan_t
{
	vec3_t normal;
	float d;
} plan_t;


typedef enum { PLAN_NEAR,PLAN_FAR,PLAN_RIGHT, PLAN_LEFT, PLAN_TOP, PLAN_BOTTOM } Plan_types;

typedef plan_t frustrum_t[6];





void COLL_InitUnitCube(void);
Intersection_test COLL_CheckAgainstUnitCube(prec_face_t* face);


static float COLL_distPointPlan(vec3_t point, plan_t* plan);

Intersection_test COLL_CheckFaceAgainstPlan(prec_face_t* face,plan_t* plan);


void COLL_GenerateFrustrum(matrix_t pvm,frustrum_t frustrum);
Intersection_test COLL_CheckBoxAgainstFrustrum(bbox_t box, frustrum_t frustrum);

void COLL_CheckEnemies(void);
void COLL_CheckPlayers(void);


#endif