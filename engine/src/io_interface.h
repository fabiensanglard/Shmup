#ifndef io_interface_h
#define io_interface_h


typedef enum  {IO_EVENT_BEGAN,IO_EVENT_MOVED,IO_EVENT_ENDED } event_type_e;

typedef struct io_event_s
{
	// Used to be an array of int (Worked well on windows,macosx and iphone).
	// but Android return coordinates as float....
	float position[2];
	float previousPosition[2];

	 event_type_e type;
} io_event_s;


//Expect event position in windows active+passive surface coordinates system.
void IO_PushEvent(io_event_s* event);

void IO_Init(void);

#endif