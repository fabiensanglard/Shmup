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
 *  lexer.h
 *  dEngine
 *
 *  Created by fabien sanglard on 24/09/09.
 *  Copyright 2009 Memset software Inc. All rights reserved.
 *
 */

#ifndef DE_LEXER
#define DE_LEXER

#include "filesystem.h"

void LE_init( filehandle_t *textFile);
void LE_skipWhiteSpace(void);
void LE_SkipRestOfLine(void);
char* LE_readToken(void);
char* LE_getCurrentToken();
float LE_readReal(void);
int LE_hasMoreData(void);
void LE_SetWhiteCharValue(char c, char value);

void LE_popLexer();
void LE_pushLexer();

#endif
