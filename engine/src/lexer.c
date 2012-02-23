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
#include "log.h"

#define STACK_SIZE 6
#define MAX_TOKEN_SIZE 256

filehandle_t fileParsed;
char token[MAX_TOKEN_SIZE];

#define TOKEN_BUFFER_HAS_ONE_MORE_SLOT (tokenChar < &token[MAX_TOKEN_SIZE-1])
#define TOKEN_BUFFER_HAS_TWO_MORE_SLOT (tokenChar < &token[MAX_TOKEN_SIZE-2])

unsigned int stackPointer=0;
filehandle_t filesStack[STACK_SIZE];


char whiteCharacters[128] ;






void LE_popLexer()
{
	stackPointer--;
	fileParsed = filesStack[stackPointer] ;
	
	
}

void LE_pushLexer()
{
	filesStack[stackPointer] = fileParsed;
	stackPointer++ ;
}

//Move ahead until after the 
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

 int  prtCurrentIsWhiteChar(void)
{
	return whiteCharacters[*fileParsed.ptrCurrent];
}

//Is the at least one character to be consumed after fileParsed.ptrCurrent .
int LE_hasMoreData(void)
{
	return (fileParsed.ptrCurrent < fileParsed.ptrEnd-1);
}

char LE_Peek(void)
{
	if (!LE_hasMoreData())
		return 0;

	return *(fileParsed.ptrCurrent+1);
}

void LE_init(filehandle_t* textFile)
{
	fileParsed = *textFile;	
	
	
	memset(whiteCharacters,0,128*sizeof(char));
	
	whiteCharacters['\0'] = 1;
	whiteCharacters[' '] = 1;
	whiteCharacters['	'] = 1;
//	whiteCharacters[','] = 1;
	whiteCharacters['('] = 1;
	whiteCharacters[')'] = 1;
	whiteCharacters['\r'] = 1;
	whiteCharacters['\n'] = 1;
	whiteCharacters['*'] = 1;
	whiteCharacters[':'] = 1;
	whiteCharacters[';'] = 1;


}


void LE_skipWhiteSpace(void)
{
	char previousChar=0;

	while(LE_hasMoreData())
	{
		

		//SQL comment style (just because I wanted those too :P !
		if (*fileParsed.ptrCurrent == '#')
		{
			LE_SkipRestOfLine();
			//LE_skipWhiteSpace();	
			//return;
		}
		
		//Legacy C comment style //
		if (*fileParsed.ptrCurrent == '/')
		{
			if(LE_Peek() == '/')
			{
				fileParsed.ptrCurrent++;
				LE_SkipRestOfLine();
			}
			
		}
		
		//C++ comment style /* */
		if (*fileParsed.ptrCurrent == '/')
		{
			if(LE_Peek() ==  '*')
			{
				fileParsed.ptrCurrent++;
				

				//Skip until */ is found
				while (LE_hasMoreData())
				{
					previousChar = *fileParsed.ptrCurrent++;
					if ((*fileParsed.ptrCurrent == '/') && (previousChar == '*'))
					{
						fileParsed.ptrCurrent++;
						break;
					}
				}
			}
		}

		if (prtCurrentIsWhiteChar())
		{
			fileParsed.ptrCurrent++;
			continue;
		}
		else{
			// We are currently pointing to a non whitespace character. This probably needs to be 
			// turned into a token.
			return;
		}
	}
}

char* LE_readToken(void)
{
	char* tokenChar = NULL;
    //tokenChat always point to the slot that will be written. It is used as a "write and move" pointer (as opposite to "move and write).

	LE_skipWhiteSpace();
	
	tokenChar = token;
	*tokenChar = '\0';
	
	if (!LE_hasMoreData())
		return token;

	//String literal
	if (*fileParsed.ptrCurrent == '"')
	{
		*tokenChar++ = *fileParsed.ptrCurrent++;
		while(TOKEN_BUFFER_HAS_TWO_MORE_SLOT && LE_hasMoreData() && *fileParsed.ptrCurrent != '"')
		{
			//			previousChar = *fileParsed.ptrCurrent;
			*tokenChar++ = *fileParsed.ptrCurrent++;
		}
		
		fileParsed.ptrCurrent++;
		
		*tokenChar++ = '"';
	}
	else
	{
		while(!prtCurrentIsWhiteChar() && LE_hasMoreData() && TOKEN_BUFFER_HAS_ONE_MORE_SLOT  )
			*tokenChar++ = *fileParsed.ptrCurrent++;
	}
	
	*tokenChar++ = '\0';

//	printf("%s\n",token);

	return token;
}

float LE_readReal(void)
{
	LE_readToken();
	
	return (float)atof(token);
}

char* LE_getCurrentToken()
{
	//Log_Printf("LE_getCurrentToken = %s.\n",token);
	return token;
}



void LE_cleanUpDoubleQuotes(char* string)
{
	char* cursor;
	size_t i;
	
	cursor = string;
	
	for(i=0 ; i < strlen(string) ; i++)
	{
		
		if (string[i] != '"') 
			*cursor++ = string[i];
	}
	
	*cursor = '\0';
}