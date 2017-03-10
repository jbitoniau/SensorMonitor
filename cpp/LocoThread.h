#pragma once 

namespace Loco
{

class Thread
{
public:
	static void sleep( unsigned int milliseconds );

	// On Windows it's not possible to precisely go to sleep for less than a 1ms.
	// We work around that by going to sleep for 0 ms when the value is less than 1000us.
	// This tells the system to put the thread to sleep as soon as possible and wake it up
	// also as soon as possible.
	
	// Need to double check this particularly on Raspberry Pi...
	// static void usleep( unsigned int microseconds );	
};

}