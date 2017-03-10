#pragma once

/*	
	The following preprocessor section defines the LOCO_PLATFORM symbol.
	It can either be:
	- LOCO_PLATFORM_WINDOWS (either 32 bits or 64 bits)
	- LOCO_PLATFORM_IOS (on iOS device like iPhone, iPad and on the iPhone simulator)
	- LOCO_PLATFORM_MACOS
	- LOCO_PLATFORM_LINUX
	- LOCO_PLATFORM_UNKNOWN

	At compilation, tests can be done on the platform like this:

		#if LOCO_PLATFORM == LOCO_PLATFORM_WINDOW
			...do stuff here...
		#endif

	The LOCO_PLATFORM_NAME symbol can be used to get the name of the LOCO_PLATFORM 
	as a string literal that can be manipulated by code.	

	All this is inspired from:
	http://stackoverflow.com/questions/5919996/how-to-detect-reliably-mac-os-x-ios-linux-windows-in-c-preprocessor
	http://stackoverflow.com/questions/4605842/how-to-identify-platform-compiler-from-preprocessor-macros
	http://stackoverflow.com/questions/3781520/how-to-test-if-preprocessor-symbol-is-defined-but-has-no-value
	http://www.cprogramming.com/tutorial/cpreprocessor.html

	Note:
	Need to have a look at this http://sourceforge.net/p/predef/wiki/OperatingSystems/ 
	It seems much more complete!
*/
#define LOCO_PLATFORM_WINDOWS	0
#define LOCO_PLATFORM_IOS		1
#define LOCO_PLATFORM_MACOS		2
#define LOCO_PLATFORM_LINUX		3
#define LOCO_PLATFORM_UNKNOWN	4

#ifdef _WIN64
	#define LOCO_PLATFORM LOCO_PLATFORM_WINDOWS
#elif _WIN32
	#define LOCO_PLATFORM LOCO_PLATFORM_WINDOWS
#elif __APPLE__
    #include "TargetConditionals.h"
    #if TARGET_OS_IPHONE==1
		#define LOCO_PLATFORM LOCO_PLATFORM_IOS
    #elif TARGET_IPHONE_SIMULATOR==1
        #define LOCO_PLATFORM LOCO_PLATFORM_IOS
    #elif TARGET_OS_MAC==1
        #define LOCO_PLATFORM LOCO_PLATFORM_MACOS
    #else
        #define LOCO_PLATFORM LOCO_PLATFORM_UNKNOWN
    #endif
#elif __linux
    #define LOCO_PLATFORM LOCO_PLATFORM_LINUX
#else
	#define LOCO_PLATFORM LOCO_PLATFORM_UNKNOWN
#endif

#if LOCO_PLATFORM == LOCO_PLATFORM_WINDOWS
	#define LOCO_PLATFORM_NAME "Windows"
#elif LOCO_PLATFORM == LOCO_PLATFORM_IOS
	#define LOCO_PLATFORM_NAME "iOS"
#elif LOCO_PLATFORM == LOCO_PLATFORM_MACOS
	#define LOCO_PLATFORM_NAME "MacOS"
#elif LOCO_PLATFORM == LOCO_PLATFORM_LINUX
	#define LOCO_PLATFORM_NAME "Linux"
#elif LOCO_PLATFORM == LOCO_PLATFORM_UNKNOWN
	#define LOCO_PLATFORM_NAME "Unknown"
#endif

namespace Loco
{

class Platform
{
public:
	enum EEndianness
	{
		BigEndian,		
		LittleEndian
	};

	static const char* getName() 		{ return LOCO_PLATFORM_NAME; }
	static EEndianness getEndianness()	{ return Platform::mEndianness; }

private:
	Platform();
	static Platform mPlatform;
	static EEndianness mEndianness;
};

}