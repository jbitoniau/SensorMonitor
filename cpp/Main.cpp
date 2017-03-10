#include <stdio.h>
#include <string>
#include "LocoTime.h"
#include "UDPSocket.h"

int main( int argc, char* argv[] )
{
	;
		
	unsigned int t = Loco::Time::getTimeAsMilliseconds();
	printf("%d> sending data\n", t);

	Loco::UDPSocket socket( 8282 );
	std::string data = "hello world!!!";
	int	ret = socket.send( data.c_str(), data.size(), "127.0.0.1", 8181 );
	printf("ret = %d\n", ret);

	return 0;
}