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
//
//  main.m
//  dEngine
//
//  Created by fabien sanglard on 09/08/09.
//  Copyright Memset software Inc 2009. All rights reserved.
//

#import <UIKit/UIKit.h>

int main(int argc, char *argv[]) {
    
	char rd[256];
	const char* wd;
	
	
	NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];
	
	strcpy( rd, argv[0] );
	int len = strlen( rd );
	for( int i = len-1; i >= 0; i-- ) {
		if ( rd[i] == '/' ) {
			rd[i] = 0;
			break;
		}
		rd[i] = 0;
	}
	setenv( "RD", rd, 1 );
	
	wd = [[NSSearchPathForDirectoriesInDomains(  NSDocumentDirectory, NSUserDomainMask, YES) objectAtIndex:0] UTF8String];
	
	setenv( "WD", wd, 1 );
	

    int retVal = UIApplicationMain(argc, argv, nil, nil);
    [pool release];
    return retVal;
}
