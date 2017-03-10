#include "LocoThread.h"

#ifdef _WIN32
	#include <assert.h>
	#include <Windows.h>
#else
	#include <time.h>
#endif

namespace Loco
{

#ifdef _WIN32
class ThreadInitializer
{
public:
	ThreadInitializer()
	{
		// Sets the minimum timer resolution to the minimum available on this machine
		// This is used by Windows for its Sleep() method. On some machine, if we don't do
		// that, the minimum time slice for a sleep can be as long as 16ms!
		// http://www.eggheadcafe.com/software/aspnet/31952897/high-resolution-sleep.aspx
		TIMECAPS timeCaps;
		memset( &timeCaps, 0, sizeof(timeCaps) );
		MMRESULT res = timeGetDevCaps( &timeCaps, sizeof(timeCaps) );
		assert(res==TIMERR_NOERROR);
		mTimePeriodDefined = timeCaps.wPeriodMin;
		res = timeBeginPeriod( mTimePeriodDefined );
		assert(res==TIMERR_NOERROR);
	}

	~ThreadInitializer()
	{
		MMRESULT res = timeEndPeriod( mTimePeriodDefined );
		assert(res==TIMERR_NOERROR);
	}

private:
	UINT						mTimePeriodDefined;
	static ThreadInitializer	mThreadInitializer;
};

ThreadInitializer ThreadInitializer::mThreadInitializer;
#endif

void Thread::sleep( unsigned int milliseconds )
{
#ifdef _WIN32
	::Sleep( milliseconds );
#else
	struct timespec l_TimeSpec;
	l_TimeSpec.tv_sec = milliseconds / 1000;
	l_TimeSpec.tv_nsec = (milliseconds % 1000) * 1000000;
	struct timespec l_Ret;
	nanosleep(&l_TimeSpec,&l_Ret);
#endif
}
/*
void Thread::usleep( unsigned int microseconds )
{
#ifdef _WIN32
	unsigned int milliseconds = microseconds / 1000;
	::Sleep( milliseconds );
#else
	usleep( microseconds );
#endif
}*/

}