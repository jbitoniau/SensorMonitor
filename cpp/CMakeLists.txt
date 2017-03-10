CMAKE_MINIMUM_REQUIRED( VERSION 2.8 )

PROJECT( SensorSender )

SET	(	SOURCES
		Platform.h
		Platform.cpp
		LocoTime.h
		LocoTime.cpp
		UDPSocket.h
		UDPSocket.cpp
		Main.cpp
	)

ADD_EXECUTABLE( ${PROJECT_NAME} ${SOURCES} )
