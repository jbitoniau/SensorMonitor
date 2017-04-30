#include "LocoTime.h"

#include "Platform.h"

#if LOCO_PLATFORM==LOCO_PLATFORM_WINDOWS
	#include <Windows.h>
#elif LOCO_PLATFORM==LOCO_PLATFORM_LINUX
	#include <cstddef>		// For NULL
	//#include <sys/time.h>
	#include <time.h>
#elif LOCO_PLATFORM==LOCO_PLATFORM_MACOS
	#include <mach/clock.h>
	#include <mach/mach.h>
#endif

#include <assert.h>

#include <stdio.h>

namespace Loco
{
	
#if LOCO_PLATFORM==LOCO_PLATFORM_WINDOWS
/*
	TimeInternals (Windows platform)

	On Windows, there are many time functions available.
	- the standard ftime function (sys/timeb.h). Millisecond resolution but 15ms accuracy
	- GetSystemTimeAsFileTime(). Millisecond resolution but 15ms accuracy
	- GetTickCount(). Millisecond resolution but up to 50 ms accuracy (to double check)
	The only high resolution/accuracy one I found is the Performance counter but it seems not 
	very reliable on multi-core machines see  http:www.virtualdub.org/blog/pivot/entry.php?id=106
	Anyway, I'm using it for now. I think this can be improved: 
	http:msdn.microsoft.com/en-us/magazine/cc163996.aspx
*/
class TimeInternals
{
public:
	TimeInternals();

	unsigned long long int	getTickFrequency() const		{ return mTickFrequency; }
	unsigned long long int	getTickCount() const;
	unsigned int			getTimeAsMilliseconds() const;
	
	static TimeInternals&	getInstance()					{ return mTimeInternals; }
	
private:
	static unsigned long long int internalGetTickFrequency();
	static unsigned long long int internalGetTickCount();
	
	unsigned long long int	mTickFrequency;
	unsigned long long int	mInitialTickCount;
	
	static TimeInternals	mTimeInternals;
};

TimeInternals TimeInternals::mTimeInternals;

TimeInternals::TimeInternals()
{
	assert(false);		// NEED TO REVISIT THIS SO MS VALUE RETURNED IS SINCE EPOCH
	mTickFrequency = internalGetTickFrequency();	// Cache the tick frequency as it doesn't change during the application lifetime
	mInitialTickCount = internalGetTickCount();
}

unsigned long long int TimeInternals::getTickCount() const
{ 
	unsigned long long int tickCount = internalGetTickCount();
	tickCount -= mInitialTickCount;
	return tickCount;
}

unsigned int TimeInternals::getTimeAsMilliseconds() const 
{
	unsigned int time = static_cast<unsigned int>( (getTickCount()*1000) / getTickFrequency() );
	return time;
}

unsigned long long int TimeInternals::internalGetTickFrequency()
{ 
	LARGE_INTEGER frequency;
	QueryPerformanceFrequency(&frequency);
	return frequency.QuadPart; 
}

unsigned long long int TimeInternals::internalGetTickCount()
{
	LARGE_INTEGER counter;
	QueryPerformanceCounter(&counter);
	return counter.QuadPart;
}

#elif LOCO_PLATFORM==LOCO_PLATFORM_LINUX

/*
	TimeInternals (Linux)

	This implementation relies on the monotonic version of clock_gettime() function.
	Changing the system date/time won't affect this time.

	See http://code-factor.blogspot.fr/2009/11/monotonic-timers.html
*/
class TimeInternals
{
public:
	TimeInternals();

	unsigned int			getTimeAsMilliseconds() const;
	
	static TimeInternals&	getInstance()					{ return mTimeInternals; }
	
private:
	struct timespec			mInitialTime;

	static TimeInternals	mTimeInternals;
};

TimeInternals TimeInternals::mTimeInternals;

TimeInternals::TimeInternals()
{
	//int ret = clock_gettime( CLOCK_MONOTONIC, &mInitialTime );
	//assert( ret==0 );
}

unsigned int TimeInternals::getTimeAsMilliseconds() const 
{
	struct timespec theTime;
	int ret = clock_gettime( CLOCK_MONOTONIC, &theTime );
	assert( ret==0 );

	time_t numSeconds = theTime.tv_sec;
	long numNanoSeconds = theTime.tv_nsec;	// 1ns = 10^-9 seconds
	
/*	unsigned int milliseconds = static_cast<unsigned int>(numSeconds - mInitialTime.tv_sec) * 1000 + 
								static_cast<unsigned int>(numNanoSeconds / 1000000) - 
								static_cast<unsigned int>(mInitialTime.tv_nsec / 1000000);
*/
	unsigned int milliseconds = static_cast<unsigned int>(numSeconds) * 1000 + 
								static_cast<unsigned int>(numNanoSeconds / 1000000); 

	return milliseconds;
}

#elif LOCO_PLATFORM==LOCO_PLATFORM_MACOS

/*
	TimeInternals (Mac OS)

	http://stackoverflow.com/questions/11680461/monotonic-clock-on-osx 
	https://books.google.fr/books?id=K8vUkpOXhN4C&pg=PA523&lpg=PA523&dq=clock_get_time&source=bl&ots=OLpoQZTu-B&sig=aaHjA6UO6OykF4ViFA8o6HO2-Tc&hl=fr&sa=X&ved=0ahUKEwi92dOT0MzSAhXqAsAKHdIqBd0Q6AEIMjAD#v=onepage&q=clock_get_time&f=false
	http://web.mit.edu/darwin/src/modules/xnu/osfmk/man/host_get_clock_service.html
*/
class TimeInternals
{
public:
	TimeInternals();

	unsigned int			getTimeAsMilliseconds() const;
	
	static TimeInternals&	getInstance()					{ return mTimeInternals; }
	
private:
	static TimeInternals	mTimeInternals;
};

TimeInternals TimeInternals::mTimeInternals;

TimeInternals::TimeInternals()
{
}

unsigned int TimeInternals::getTimeAsMilliseconds() const 
{
	clock_serv_t cclock;			
	mach_timespec_t mts;
	host_get_clock_service(mach_host_self(), SYSTEM_CLOCK, &cclock);	// Maybe doing this in ctor
	clock_get_time(cclock, &mts);
	mach_port_deallocate(mach_task_self(), cclock);						// and this in destructor is OK?

	unsigned int numSeconds = mts.tv_sec;
	clock_res_t numNanoseconds = mts.tv_nsec;
	unsigned int milliseconds = static_cast<unsigned int>(numSeconds * 1000) + 
								static_cast<unsigned int>(numNanoseconds / 1000000);
	return milliseconds;
}

/*
	http://stackoverflow.com/questions/30095439/how-do-i-get-system-up-time-in-milliseconds-in-c
	Check this out:
	#include <time.h>
#include <errno.h>
#include <sys/sysctl.h>
// ...
std::chrono::milliseconds uptime(0u);
struct timeval ts;
std::size_t len = sizeof(ts);
int mib[2] = { CTL_KERN, KERN_BOOTTIME };
if (sysctl(mib, 2, &ts, &len, NULL, 0) == 0)
{
  uptime = std::chrono::milliseconds(
    static_cast<unsigned long long>(ts.tv_sec)*1000ULL + 
    static_cast<unsigned long long>(ts.tv_usec)/1000ULL
  );
}
*/

#endif

/*
	Time
*/
unsigned int Time::getTimeAsMilliseconds()
{
	unsigned int milliseconds = TimeInternals::getInstance().getTimeAsMilliseconds();
	return milliseconds;
}

}
