#include <stdio.h>
#include <string>
#include <cmath>

#include "LocoTime.h"
#include "LocoThread.h"
#include "UDPSocket.h"

int main( int argc, char* argv[] )
{
	printf("Sending data\n");

	Loco::UDPSocket socket( 8282 );

	//for ( int i=0; i<100; i++ )
	while (true)
	{
		float k = static_cast<float>( Loco::Time::getTimeAsMilliseconds() % 3000 ) / 3000.f;
		float v = std::sin(k  * M_PI * 2) * 30 + 60;
		//printf("%f\n", v);
		Loco::Thread::sleep( 20 );
		int	ret = socket.send( reinterpret_cast<const char*>(&v), sizeof(v), "127.0.0.1", 8181 );
	}

	return 0;
}