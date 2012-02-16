#include "io_interface.h"
#include "menu.h"
#include "dEngine.h"
#include "timer.h"

int lastTouchBegan=0;

#define SQUARE(X) ((X)*(X))


float commScale[2]; 

void IO_Init(void){
	commScale[X] = SS_W/ (float)renderer.viewPortDimensions[VP_WIDTH];
	commScale[Y] = SS_H/ (float)renderer.viewPortDimensions[VP_HEIGHT];
}

void IO_PushEvent(io_event_s* event){
	


	
	
	touch_t *touch, *t2;
	int		minDist = 64 * 64  ;	// allow up to 64 unit moves to be drags
	int		minIndex = -1;

	//Button analysis variables
	int     i;
	int     dist;

	//Buttons in shmup
	int numButton;
	touch_t* currentTouchSet;





	if (engine.menuVisible)
	{
		numButton = MENU_GetNumButtonsTouches();
		currentTouchSet = MENU_GetCurrentButtonTouches();
	}
	else 
	{
		numButton = NUM_BUTTONS;
		currentTouchSet = touches;
	}
	
	if (engine.menuVisible || engine.controlMode == CONTROL_MODE_VIRT_PAD)
	{
	
       // CGPoint touchLocation = [myTouch locationInView:nil];
		
		//Transforming from whatever screen resolution we have to the original iPHone 320*480
		event->position[X] = ( event->position[X] - renderer.viewPortDimensions[VP_X] ) * commScale[X] ;//* renderer.resolution ;
		event->position[Y] = ( event->position[Y] - renderer.viewPortDimensions[VP_Y] ) * commScale[Y] ;//* renderer.resolution;


		// find which one it is closest to
		t2 = currentTouchSet;
		for (i  = 0 ; i < numButton ; i++ ) 
		{
			
			dist = SQUARE( t2->iphone_coo_SysPos[X] - event->position[X] )  + SQUARE( t2->iphone_coo_SysPos[Y] - event->position[Y] ) ;
			
			
			if ( dist < minDist ) {
				minDist = dist;
				minIndex = i;
				touch = t2;
			}
		//	printf("button:%d x=%hd, y =%.hd. dist=%.2f, minDist=%.2f\n",i,t2->iphone_coo_SysPos[X],t2->iphone_coo_SysPos[Y],(float)dist,(float)minDist);
			t2++;
		}
		
		if ( minIndex != -1 ) 
		{
			//printf("HIT ! %d.\n",minIndex);
			if (event->type == IO_EVENT_ENDED) 
			{
				//touch->down = 0;
				//printf("%d UP\n",minIndex);
                touch->down = 1;
				touch->dist[X] = MIN(1,(event->position[X] - touches[minIndex].iphone_coo_SysPos[X])/touches[minIndex].iphone_size);
				touch->dist[Y] = MIN(1,(touches[minIndex].iphone_coo_SysPos[Y] - event->position[Y])/touches[minIndex].iphone_size);
			}
			else 
			{
				if (event->type == IO_EVENT_BEGAN) 
				{
				
                }
			}
		}
	
	}
	else
	{
		
		
		
			//CGPoint touchLocation = [myTouch locationInView:nil];
			//CGPoint prevTouchLocation = [myTouch previousLocationInView:nil];
			
			//Transforming from whatever screen resolution we have to the original iPHone 320*480
			event->position[X] = ( event->position[X]- renderer.viewPortDimensions[VP_X] ) * commScale[X] ;//* renderer.resolution ;
			event->position[Y] = ( event->position[Y]- renderer.viewPortDimensions[VP_Y] ) * commScale[Y] ;//* renderer.resolution;
			
			
			
			
			if (event->type == IO_EVENT_ENDED) 
			{
				//if (touchCount == 1) //Last finger ended
					touches[BUTTON_FIRE].down = 0;
			}
			else 
			{
			 
				
			
				if (event->type == IO_EVENT_MOVED) 
				{
                    
                    event->previousPosition[X] = ( event->previousPosition[X]- renderer.viewPortDimensions[VP_X] ) * commScale[X] ;//* renderer.resolution ;
                    event->previousPosition[Y] = ( event->previousPosition[Y]- renderer.viewPortDimensions[VP_Y] ) * commScale[Y] ;//* renderer.resolution;
                    
                    
					//printf("m\n");
					touches[BUTTON_MOVE].down = 1;
                    //Used to be +80 for X and -80 for Y
					touches[BUTTON_MOVE].dist[X] = (event->position[X] - event->previousPosition[X])*80/(float)320;
					touches[BUTTON_MOVE].dist[Y] = (event->position[Y] - event->previousPosition[Y])*-80/(float)480;
					
				}
				if (event->type == IO_EVENT_BEGAN)
				{
					int currTime = E_Sys_Milliseconds();
					if (currTime-lastTouchBegan < 200)
						touches[BUTTON_GHOST].down = 1;
					
					lastTouchBegan = currTime ;
					
					touches[BUTTON_FIRE].down = 1;
				}
				
				
			}
			
		
	}
	
	
	
	

}