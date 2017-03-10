#pragma once 

namespace Loco
{

class Time
{
public:
	// Returns the time in milliseconds ellapsed since an arbitrary time-reference.
	// The time reference here is something close in the past, like the start of 
	// the computer or the application itself for example. This is because the 
	// returned value here is a 32 bit integer. Exceeding about 49.71 days means 
	// looping back to 0 again.
	static unsigned int getTimeAsMilliseconds();
};

}
