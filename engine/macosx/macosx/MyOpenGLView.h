/*
SHMUP is a 3D Shoot 'em up game inspired by Treasure Ikaruga

This file is part of SCHMUP.

SCHMUP is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

SCHMUP is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with SCHMUP.  If not, see <https://www.gnu.org/licenses/>.
*/

//
//  MyOpenGLView.h
//  macosx
//
//  Created by fabien sanglard on 12-01-31.
//  Copyright (c) 2012 Memset Software. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface MyOpenGLView : NSOpenGLView
{
     NSTimer* timer;
}

- (void) drawRect: (NSRect) bounds;
@end
