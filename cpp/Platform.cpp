#include "Platform.h"

#include <cstdint>

namespace Loco
{

Platform::EEndianness Platform::mEndianness = Platform::LittleEndian;

Platform::Platform()
{
	volatile std::uint32_t i=0x01234567;
    if ( (*((std::uint8_t*)(&i))) == 0x67 )
		Platform::mEndianness = LittleEndian;
	else
		Platform::mEndianness = BigEndian;
}

}