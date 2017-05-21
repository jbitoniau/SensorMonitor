#include <cstdint>
#include <stdio.h>
#include <string>
#include <cmath>

#include "LocoTime.h"
#include "LocoThread.h"
#include "UDPSocket.h"

#include "MPU6050/MPU6050.h"
#include "HMC5883L/HMC5883L.h"
#include "MS561101BA/MS561101BA.h"

extern "C" 
{
	#include "pwm.h"
}

/*
	MPU6050
*/
double getAccelerometerHalfScaleRange( MPU6050& mpu6050 )
{
	uint8_t scaleCode = mpu6050.getFullScaleAccelRange();
	switch ( scaleCode )
	{
		case MPU6050_ACCEL_FS_2 : return 2.0;
		case MPU6050_ACCEL_FS_4 : return 4.0;
		case MPU6050_ACCEL_FS_8 : return 8.0;
		case MPU6050_ACCEL_FS_16 : return 16.0;
	}
	printf("getAccelerometerHalfScaleRange unsupported value returned\n");
	return 2.0;
}

double getGyroscopeHalfScaleRange( MPU6050& mpu6050 )
{
	uint8_t scaleCode = mpu6050.getFullScaleGyroRange();
	switch ( scaleCode )
	{
		case MPU6050_GYRO_FS_250 : return 250.0;
		case MPU6050_GYRO_FS_500 : return 500.0;
		case MPU6050_GYRO_FS_1000 : return 1000.0;
		case MPU6050_GYRO_FS_2000 : return 2000.0;
	}
	printf("getGyroscopeHalfScaleRange unsupported value returned\n");
	return 250.0;
}

struct DataSample {
	double  		accelerationX;			// In Gs
	double  		accelerationY;
	double  		accelerationZ;
	double  		angularSpeedX;			// In degrees per sec
	double  		angularSpeedY;
	double  		angularSpeedZ;
	float 	 		temperature;			// In degrees celsius

	double  		magneticHeadingX;		// In gauss. In Probably change that to int16
	double  		magneticHeadingY;
	double  		magneticHeadingZ;

	float         	temperature2;			// In degrees celsius
	float         	pressure;				// In HPa
	
	std::uint32_t	timestamp;
};

DataSample getDataSample( 
	MPU6050& mpu6050, double accelerometerHalfScaleRange, double gyroscopeHalfScaleRange,
	HMC5883L& hmc5883l, MS561101BA& ms561101ba )
{
	DataSample dataSample; 

	// MPU6050
	int16_t ax, ay, az;
	int16_t gx, gy, gz;
	mpu6050.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);		// This takes 1 or 2 ms
	dataSample.accelerationX = static_cast<double>(ax) * accelerometerHalfScaleRange / 32768.0;
	dataSample.accelerationY = static_cast<double>(ay) * accelerometerHalfScaleRange / 32768.0;
	dataSample.accelerationZ = static_cast<double>(az) * accelerometerHalfScaleRange / 32768.0;
	dataSample.angularSpeedX = static_cast<double>(gx) * gyroscopeHalfScaleRange / 32768.0;
	dataSample.angularSpeedY = static_cast<double>(gy) * gyroscopeHalfScaleRange / 32768.0;
	dataSample.angularSpeedZ = static_cast<double>(gz) * gyroscopeHalfScaleRange / 32768.0;

	int16_t t = mpu6050.getTemperature();
	dataSample.temperature = static_cast<float>(t)/340.f + 36.53f;
		
	// HMC5883L
	int16_t mx = 0;
	int16_t my = 0;
	int16_t mz = 0;
	hmc5883l.getHeading(&mx, &my, &mz);						// This takes 1 or 2 ms
	dataSample.magneticHeadingX = static_cast<double>(mx);
	dataSample.magneticHeadingY = static_cast<double>(my);
	dataSample.magneticHeadingZ = static_cast<double>(mz);

	// MS561101BA
	float temperature = 0.f;			
	float pressure = 0.f;
	ms561101ba.readValues( &pressure, &temperature );		// This takes 4 or 5 ms
	dataSample.temperature2 = temperature;
	dataSample.pressure = pressure;
	
	// Timestamp
	dataSample.timestamp = Loco::Time::getTimeAsMilliseconds();

	return dataSample;
}

int serializeDataSample( const DataSample& dataSample, char* buffer )
{
	int int32Size = sizeof( std::uint32_t );
	int floatSize = sizeof(float);
	int doubleSize = sizeof(double);

	int offset = 0;
	memcpy( buffer+offset, reinterpret_cast<const char*>(&dataSample.accelerationX), doubleSize ); offset+=doubleSize;
	memcpy( buffer+offset, reinterpret_cast<const char*>(&dataSample.accelerationY), doubleSize ); offset+=doubleSize;
	memcpy( buffer+offset, reinterpret_cast<const char*>(&dataSample.accelerationZ), doubleSize ); offset+=doubleSize;
	memcpy( buffer+offset, reinterpret_cast<const char*>(&dataSample.angularSpeedX), doubleSize ); offset+=doubleSize;
	memcpy( buffer+offset, reinterpret_cast<const char*>(&dataSample.angularSpeedY), doubleSize ); offset+=doubleSize;
	memcpy( buffer+offset, reinterpret_cast<const char*>(&dataSample.angularSpeedZ), doubleSize ); offset+=doubleSize;
	memcpy( buffer+offset, reinterpret_cast<const char*>(&dataSample.temperature), floatSize ); offset+=floatSize;
	
	memcpy( buffer+offset, reinterpret_cast<const char*>(&dataSample.magneticHeadingX), doubleSize ); offset+=doubleSize;
	memcpy( buffer+offset, reinterpret_cast<const char*>(&dataSample.magneticHeadingY), doubleSize ); offset+=doubleSize;
	memcpy( buffer+offset, reinterpret_cast<const char*>(&dataSample.magneticHeadingZ), doubleSize ); offset+=doubleSize;
	
	memcpy( buffer+offset, reinterpret_cast<const char*>(&dataSample.temperature2), floatSize ); offset+=floatSize;
	memcpy( buffer+offset, reinterpret_cast<const char*>(&dataSample.pressure), floatSize ); offset+=floatSize;
	
	memcpy( buffer+offset, reinterpret_cast<const char*>(&dataSample.timestamp), int32Size ); offset+=int32Size;
	return offset;
}

int main( int argc, char* argv[] )
{
	// Fatal errors in the rpio-pwm lib will not cause it to exit the program
	// The function will simply return EXIT_FAILURE and a description of the
	// problem can be obtained using get_error_message()
	set_softfatal(1);
	
	// Only report errors (not simple debug messages)
	set_loglevel(LOG_LEVEL_ERRORS);
		
	int pulseWidthIncrementInUs = 10; 
	int hardware = DELAY_VIA_PWM;
	int ret = setup(pulseWidthIncrementInUs, hardware);
	if ( ret!=EXIT_SUCCESS )
	{
		printf("Problem RPIOPWMProvider: %s\n", get_error_message());
	}

	unsigned int channelSubcycleTimeInUs = 20000;
	int channelIndex = 0;
	ret = init_channel(channelIndex, channelSubcycleTimeInUs);		
	printf("RPIOPWM: channelSubcycleTimeInUs set to %d us\n", channelSubcycleTimeInUs);
	if ( ret!=EXIT_SUCCESS )
	{
		printf("Problem RPIOPWM: %s\n", get_error_message());
	}

	/*
	// 8 GPIOs to use for driving servos
	int RPIOPWMDevice::mValidGPIOs[] = {
		//GPIO#   Pin#
	    4,     // P1-7
	    17,    // P1-11
	    18,    // P1-12
	    21,    // P1-13
	    22,    // P1-15
	    23,    // P1-16
	    24,    // P1-18
	    25,    // P1-22
	};
	*/
	int gpio = 25;
	{
		int widthInUs = 1000;
		printf("%d\n", widthInUs);
		int widthInIncrements = widthInUs / pulseWidthIncrementInUs;
		ret = add_channel_pulse( channelIndex, gpio, 0, widthInIncrements);
		if ( ret!=EXIT_SUCCESS )
		{
			printf("Problem internalSetPulseWidthInUs: %s\n", get_error_message());
		}
	}
	Loco::Thread::sleep( 2000 );

	{
		int widthInUs = 1200;
		printf("%d\n", widthInUs);
		int widthInIncrements = widthInUs / pulseWidthIncrementInUs;
		ret = add_channel_pulse( channelIndex, gpio, 0, widthInIncrements);
		if ( ret!=EXIT_SUCCESS )
		{
			printf("Problem internalSetPulseWidthInUs: %s\n", get_error_message());
		}
	}
	Loco::Thread::sleep( 1000 );

	{
		int widthInUs = 1400;
		printf("%d\n", widthInUs);
		int widthInIncrements = widthInUs / pulseWidthIncrementInUs;
		ret = add_channel_pulse( channelIndex, gpio, 0, widthInIncrements);
		if ( ret!=EXIT_SUCCESS )
		{
			printf("Problem internalSetPulseWidthInUs: %s\n", get_error_message());
		}
	}
	Loco::Thread::sleep( 1000 );
	{
		int widthInUs = 1000;
		printf("%d\n", widthInUs);
		int widthInIncrements = widthInUs / pulseWidthIncrementInUs;
		ret = add_channel_pulse( channelIndex, gpio, 0, widthInIncrements);
		if ( ret!=EXIT_SUCCESS )
		{
			printf("Problem internalSetPulseWidthInUs: %s\n", get_error_message());
		}
	}

	printf("Sending data\n");

	Loco::UDPSocket socket( 8282 );

	// MPU6050
	MPU6050	mpu6050(0x69);
	mpu6050.initialize();
	mpu6050.setI2CMasterModeEnabled(0);     // !!!!!!!!!!!!!
	mpu6050.setI2CBypassEnabled(1);         // !!!!!!!!!!!!!
	mpu6050.setFullScaleAccelRange(MPU6050_ACCEL_FS_4);		// in Gs
	mpu6050.setFullScaleGyroRange(MPU6050_GYRO_FS_1000);	// in deg/s
	double accelerometerHalfScaleRange = getAccelerometerHalfScaleRange(mpu6050);
	double gyroscopeHalfScaleRange = getGyroscopeHalfScaleRange(mpu6050);
    printf(mpu6050.testConnection() ? "MPU6050 connection successful" : "MPU6050 connection failed");

    // HMC5883L
    HMC5883L hmc5883l(0x1E);
	hmc5883l.initialize();
	printf(hmc5883l.testConnection() ? "HMC5883L connection successful" : "HMC5883L connection failed");
    // Note: the initial setting of the device should happen here

	// MS561101BA
	MS561101BA ms561101ba(0x77);
	ms561101ba.initialize();      // Note: this is costly on the MS5611-01BA. See if we can decrease the delay
	printf(ms561101ba.testConnection() ? "MS561101BA connection successful" : "MS561101BA connection failed");
	// Note: the initial setting of the device should happen here

int lastT, t, tsensor, tsend;

    char buffer[512];

  	int startTime = Loco::Time::getTimeAsMilliseconds();
  	int framePeriod = 20;
	
	int lastLoopIndex = static_cast<int>( std::floor( startTime/framePeriod) );

	while (true)
	{	
		
	lastT = Loco::Time::getTimeAsMilliseconds();
		DataSample dataSample = getDataSample( mpu6050, accelerometerHalfScaleRange, gyroscopeHalfScaleRange, hmc5883l, ms561101ba );
		
	t = Loco::Time::getTimeAsMilliseconds();
	tsensor = t - lastT;
	lastT = t;

		int numBytesToSend = serializeDataSample( dataSample, buffer );	
		int numBytesSent = socket.send( buffer, numBytesToSend, "127.0.0.1", 8181 );
	
	t = Loco::Time::getTimeAsMilliseconds();
	tsend = t - lastT;
//	printf( "%d  %d  \n", tsensor, tsend );
		//printf( "%d  %d    %f\n", numBytesToSend, numBytesSent, dataSample.pressure);
		
		int loopIndex = static_cast<int>( std::floor( Loco::Time::getTimeAsMilliseconds()/framePeriod) );
		while ( loopIndex==lastLoopIndex )
		{
			Loco::Thread::sleep( 1 );
			loopIndex = static_cast<int>( std::floor( Loco::Time::getTimeAsMilliseconds()/framePeriod) );
		}
		lastLoopIndex = loopIndex;
	}

	return 0;
}