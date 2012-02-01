#ifndef io_interface_h
#define io_interface_h


typedef enum  {IO_EVENT_BEGAN,IO_EVENT_MOVED,IO_EVENT_ENDED  , IO_EVENT_REQUEST_MENU} event_type_e;

typedef struct event_s
{
	int position[2];
	int previousPosition[2];
	 event_type_e type;
} event_t;


//Expect event position in windows active+passive surface coordinates system.
void IO_PushEvent(event_t* event);

void IO_Init(void);

#endif