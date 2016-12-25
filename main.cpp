#include <asm/types.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <iostream>
#include <unistd.h>
#include "joystick.h"

struct js_event 
{
	__u32 time;     /* event timestamp in milliseconds */
	__s16 value;    /* value */
	__u8 type;      /* event type */
	__u8 number;    /* axis/button number */
};

#define JS_EVENT_BUTTON         0x01    /* button pressed/released */
#define JS_EVENT_AXIS           0x02    /* joystick moved */
#define JS_EVENT_INIT           0x80    /* initial state of device */

int main()
{
	ppm_stream ppmstream(std::cout);

	joystick joy("mappings.ini", "user_trims.ini", ppmstream);
	
	// open joystick
	int fd = open("/dev/input/js0", O_RDONLY);
	
	js_event e;
	
	// read from joystick
	while(read(fd, &e, sizeof(e)) > 0)
	{
		e.type = e.type &~JS_EVENT_INIT;
		
		if (e.type == JS_EVENT_AXIS)
		{
			joy.put_axis(e.number, (float)e.value / 32768.0f);
		}
		else if (e.type == JS_EVENT_BUTTON)
		{
			joy.put_button(e.number, e.value != 0);
		}
	}
	return 0;
}
