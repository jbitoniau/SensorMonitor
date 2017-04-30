CMAKE_MINIMUM_REQUIRED( VERSION 3.0 )

PROJECT( I2CDevLib )

IF( CMAKE_SYSTEM_NAME MATCHES Linux )
	
	# Examples
	ADD_SUBDIRECTORY( MPU6050/Examples )
	ADD_SUBDIRECTORY( BMP085/Examples )
	ADD_SUBDIRECTORY( MS561101BA/Examples )

	# The source code of the different I2C device classes 
	# include "I2Cdev.h" (without path), so we add the folder 
	# containing that header to the include directories
	INCLUDE_DIRECTORIES( ${I2CDevLib_SOURCE_DIR}/I2Cdev )

	SET( SOURCES
		I2Cdev/I2Cdev.h
		I2Cdev/I2Cdev.cpp
		#LM73/LM73.h
		#LM73/LM73.cpp
		BMP085/BMP085.h
		BMP085/BMP085.cpp
		HMC5883L/HMC5883L.h
		HMC5883L/HMC5883L.cpp
		#MPR121/MPR121.h
		#MPR121/MPR121.cpp
		MPU6050/MPU6050.h
		MPU6050/MPU6050.cpp
		MS561101BA/MS561101BA.h
		MS561101BA/MS561101BA.cpp
	   )
 
	ADD_LIBRARY( ${PROJECT_NAME} STATIC ${SOURCES} )
	
	INSTALL(TARGETS ${PROJECT_NAME} EXPORT PihaInstallTargets
		LIBRARY DESTINATION lib
		ARCHIVE DESTINATION lib
		RUNTIME DESTINATION bin )
	#SET( ConfigPackageLocation lib/cmake/${PROJECT_NAME} )
	#INSTALL(EXPORT ${PROJECT_NAME}Targets 
	#	FILE ${PROJECT_NAME}Targets.cmake
	#	NAMESPACE ${TARGET_NAMESPACE}
	#	DESTINATION ${ConfigPackageLocation} )
		
ELSE()
	MESSAGE("${PROJECT_NAME} is Linux only")
ENDIF()
