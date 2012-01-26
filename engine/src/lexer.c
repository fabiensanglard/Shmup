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
 *  lexer.c
 *  dEngine
 *
 *  Created by fabien sanglard on 24/09/09.
 *  Copyright 2009 Memset software Inc. All rights reserved.
 *
 */

#include "lexer.h"


#define STACK_SIZE 6
#define MAX_TOKEN_SIZE 256

filehandle_t fileParsed;
char* token = NULL;
char* tokenChar = NULL;

unsigned int stackPointer=0;
filehandle_t filesStack[STACK_SIZE];
char tokensStack[STACK_SIZE][MAX_TOKEN_SIZE];
char* tokenCharsStack[STACK_SIZE];


char whiteCharacters[128] ;



void LE_popLexer()
{
	stackPointer--;
	fileParsed = filesStack[stackPointer] ;
	token = tokensStack[stackPointer]  ;
	tokenChar = tokenCharsStack[stackPointer] ;
}

void LE_pushLexer()
{
	filesStack[stackPointer] = fileParsed;
	
	tokenCharsStack[stackPointer] = tokenChar;
	stackPointer++ ;
	token = tokensStack[stackPointer];
	
	*token = '\0';
	tokenChar = token;
}

void LE_SkipRestOfLine(void)
{
	while (LE_hasMoreData())
	{
		if (*fileParsed.ptrCurrent == '\n')
		{
			fileParsed.ptrCurrent++;
			return;
		}
		fileParsed.ptrCurrent++;
	}
}

int prtCurrentIsWhiteChar(void)
{
	return whiteCharacters[*fileParsed.ptrCurrent];
}

void LE_init(filehandle_t* textFile)
{
	fileParsed = *textFile;	
	
	if (token == NULL)
		token = tokensStack[0];
	
	tokenChar = token;
	
	
	memset(whiteCharacters,0,128*sizeof(char));
	
	whiteCharacters[' '] = 1;
	whiteCharacters['	'] = 1;
//	whiteCharacters[','] = 1;
	whiteCharacters['('] = 1;
	whiteCharacters[')'] = 1;
	whiteCharacters['\r'] = 1;
	whiteCharacters['\n'] = 1;
	whiteCharacters['*'] = 1;
	whiteCharacters[':'] = 1;
	
}

void LE_SetWhiteCharValue(char c, char value)
{
	whiteCharacters[c] = value;
}

char previousChar=0;
void LE_skipWhiteSpace(void)
{
	if (!LE_hasMoreData())
		return;
	
	
	
	while(LE_hasMoreData() && prtCurrentIsWhiteChar())
	{
		previousChar = *fileParsed.ptrCurrent;
		fileParsed.ptrCurrent++;
		
		if (*fileParsed.ptrCurrent == '#')
		{
			LE_SkipRestOfLine();
			//LE_skipWhiteSpace();	
			//return;
		}
		
		if (*fileParsed.ptrCurrent == '/')
		{
			if((fileParsed.ptrCurrent+1) != fileParsed.ptrEnd &&  *(fileParsed.ptrCurrent+1) == '/')
			{
				fileParsed.ptrCurrent++;
				LE_SkipRestOfLine();
			}
			
		}
		
		if (*fileParsed.ptrCurrent == '*')
		{
			if(previousChar == '/')
			{
				//Skip until */ is found
				while (LE_hasMoreData())
				{
					previousChar = *fileParsed.ptrCurrent++;
					if ((*fileParsed.ptrCurrent == '/') && (previousChar == '*'))
						break;
				}
				
				if ( LE_hasMoreData())
				{
					previousChar = *fileParsed.ptrCurrent;
					fileParsed.ptrCurrent++;
					LE_skipWhiteSpace();
				}
			}
		}
	}
}

char* LE_readToken(void)
{
	LE_skipWhiteSpace();
	
	tokenChar = token;
	*tokenChar = '\0';
	
	//String literal
	if (*fileParsed.ptrCurrent == '"')
	{
		*tokenChar++ = *fileParsed.ptrCurrent++;
		while(LE_hasMoreData() && *fileParsed.ptrCurrent != '"')
		{
			//			previousChar = *fileParsed.ptrCurrent;
			*tokenChar++ = *fileParsed.ptrCurrent++;
		}
		
		fileParsed.ptrCurrent++;
		
		*tokenChar++ = '"';
		*tokenChar++ = '\0';
		return token;
	}
	else
	{
		while(LE_hasMoreData() && !prtCurrentIsWhiteChar())
			*tokenChar++ = *fileParsed.ptrCurrent++;
	}
	
	*tokenChar++ = '\0';
	
	return token;
}

float LE_readReal(void)
{
	LE_readToken();
	
	return (float)atof(token);
}

char* LE_getCurrentToken()
{
	//printf("LE_getCurrentToken = %s.\n",token);
	return token;
}


int LE_hasMoreData(void)
{
	return (fileParsed.ptrCurrent < fileParsed.ptrEnd);
}


