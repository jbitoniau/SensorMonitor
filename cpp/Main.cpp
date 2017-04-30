#include <cstdint>
#include <stdio.h>
#include <string>
#include <cmath>

#include "LocoTime.h"
#include "LocoThread.h"
#include "UDPSocket.h"

#include "MPU6050/MPU6050.h"

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
	double  		accelerationX;
	double  		accelerationY;
	double  		accelerationZ;
	double  		angularSpeedX;
	double  		angularSpeedY;
	double  		angularSpeedZ;
	float 	 		temperature;
	std::uint32_t	timestamp;
};

DataSample getDataSample( MPU6050& mpu6050, double accelerometerHalfScaleRange, double gyroscopeHalfScaleRange )
{
	DataSample dataSample; 

	int16_t ax, ay, az;
	int16_t gx, gy, gz;
	mpu6050.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
	dataSample.accelerationX = static_cast<double>(ax) * accelerometerHalfScaleRange / 32768.0;
	dataSample.accelerationY = static_cast<double>(ay) * accelerometerHalfScaleRange / 32768.0;
	dataSample.accelerationZ = static_cast<double>(az) * accelerometerHalfScaleRange / 32768.0;
	dataSample.angularSpeedX = static_cast<double>(gx) * gyroscopeHalfScaleRange / 32768.0;
	dataSample.angularSpeedY = static_cast<double>(gy) * gyroscopeHalfScaleRange / 32768.0;
	dataSample.angularSpeedZ = static_cast<double>(gz) * gyroscopeHalfScaleRange / 32768.0;

	int16_t t = mpu6050.getTemperature();
	dataSample.temperature = static_cast<float>(t)/340.f + 36.53f;
		
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
	memcpy( buffer+offset, reinterpret_cast<const char*>(&dataSample.timestamp), int32Size ); offset+=int32Size;
	return offset;
}

int main( int argc, char* argv[] )
{
	printf("Sending data\n");

	Loco::UDPSocket socket( 8282 );

	//for ( int i=0; i<100; i++ )
	MPU6050	mpu6050(0x69);
	mpu6050.initialize();
	mpu6050.setI2CMasterModeEnabled(0);     // !!!!!!!!!!!!!
	mpu6050.setI2CBypassEnabled(1);         // !!!!!!!!!!!!!

	mpu6050.setFullScaleAccelRange(MPU6050_ACCEL_FS_4);		// in Gs
	mpu6050.setFullScaleGyroRange(MPU6050_GYRO_FS_1000);	// in deg/s
	
	double accelerometerHalfScaleRange = getAccelerometerHalfScaleRange(mpu6050);
	double gyroscopeHalfScaleRange = getGyroscopeHalfScaleRange(mpu6050);

    printf(mpu6050.testConnection() ? "MPU6050 connection successful" : "MPU6050 connection failed");
    Loco::Thread::sleep( 1000 );

    char buffer[512];

	while (true)
	{	
		DataSample dataSample = getDataSample( mpu6050, accelerometerHalfScaleRange, gyroscopeHalfScaleRange );
		int numBytesToSend = serializeDataSample( dataSample, buffer );	
		int numBytesSent = socket.send( buffer, numBytesToSend, "127.0.0.1", 8181 );
		printf( "%d  %d    %f\n", numBytesToSend, numBytesSent, dataSample.angularSpeedX);
		Loco::Thread::sleep( 20 );
		//float k = static_cast<float>( Loco::Time::getTimeAsMilliseconds() % 3000 ) / 3000.f;
		//float v = std::sin(k  * M_PI * 2) * 30 + 60;
	}

	return 0;
}