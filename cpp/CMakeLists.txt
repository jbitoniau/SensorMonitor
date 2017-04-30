CMAKE_MINIMUM_REQUIRED( VERSION 2.8 )

PROJECT( SensorSender )

ADD_DEFINITIONS(-std=c++11)   # for <cstdint> on Raspberry Pi
SET	(	SOURCES
		Platform.h
		Platform.cpp
		LocoTime.h
		LocoTime.cpp
		LocoThread.h
		LocoThread.cpp
		UDPSocket.h
		UDPSocket.cpp
		Main.cpp
	)

ADD_EXECUTABLE( ${PROJECT_NAME} ${SOURCES} )
