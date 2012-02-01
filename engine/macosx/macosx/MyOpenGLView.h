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
