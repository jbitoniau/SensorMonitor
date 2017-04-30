CMAKE_MINIMUM_REQUIRED( VERSION 3.0 )

PROJECT( SensorSender )

ADD_SUBDIRECTORY( Dependencies )

# The source code of the different I2C device classes include "I2Cdev.h" (without path),
# so we add the folder containing that header to the include directories
INCLUDE_DIRECTORIES( ${I2CDevLib_SOURCE_DIR}/I2Cdev )
INCLUDE_DIRECTORIES( ${I2CDevLib_SOURCE_DIR} )

MESSAGE( ${I2CDevLib_SOURCE_DIR} )
		
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
TARGET_LINK_LIBRARIES( ${PROJECT_NAME} I2CDevLib  ) 