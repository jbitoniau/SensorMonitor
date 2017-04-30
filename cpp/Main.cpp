#include <stdio.h>
#include <string>
#include <cmath>

#include "LocoTime.h"
#include "LocoThread.h"
#include "UDPSocket.h"

#include "MPU6050/MPU6050.h"

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
	double	angularSpeedX;
	double	angularSpeedY;
	double	angularSpeedZ;
	float	temperature;
};

DataSample getDataSample( MPU6050& mpu6050, double gyroscopeHalfScaleRange )
{
	DataSample dataSample; 

	int16_t ax, ay, az;
	int16_t gx, gy, gz;
	mpu6050.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
	dataSample.angularSpeedX = static_cast<double>(gx) * gyroscopeHalfScaleRange / 32768.0;
	dataSample.angularSpeedY = static_cast<double>(gy) * gyroscopeHalfScaleRange / 32768.0;
	dataSample.angularSpeedZ = static_cast<double>(gz) * gyroscopeHalfScaleRange / 32768.0;

	int16_t t = mpu6050.getTemperature();
	dataSample.temperature = static_cast<float>(t)/340.f + 36.53f;
		
	return dataSample;
}

int serializeDataSample( const DataSample& dataSample, char* buffer )
{
	int floatSize = sizeof(float);
	int doubleSize = sizeof(double);

	int offset = 0;
	memcpy( buffer+offset, reinterpret_cast<const char*>(&dataSample.angularSpeedX), doubleSize ); offset+=doubleSize;
	memcpy( buffer+offset, reinterpret_cast<const char*>(&dataSample.angularSpeedY), doubleSize ); offset+=doubleSize;
	memcpy( buffer+offset, reinterpret_cast<const char*>(&dataSample.angularSpeedZ), doubleSize ); offset+=doubleSize;
	memcpy( buffer+offset, reinterpret_cast<const char*>(&dataSample.temperature), floatSize ); offset+=floatSize;
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
	mpu6050.setFullScaleGyroRange(MPU6050_GYRO_FS_1000);			// 1000 deg/s
	
	double gyroscopeHalfScaleRange = getGyroscopeHalfScaleRange(mpu6050);

    printf(mpu6050.testConnection() ? "MPU6050 connection successful" : "MPU6050 connection failed");
    Loco::Thread::sleep( 1000 );

    char buffer[512];

	while (true)
	{	
		DataSample dataSample = getDataSample( mpu6050, gyroscopeHalfScaleRange );
		int numBytesToSend = serializeDataSample( dataSample, buffer );	
		int numBytesSent = socket.send( buffer, numBytesToSend, "127.0.0.1", 8181 );
		printf( "%d  %d    %f\n", numBytesToSend, numBytesSent, dataSample.angularSpeedX);
		Loco::Thread::sleep( 20 );
		//float k = static_cast<float>( Loco::Time::getTimeAsMilliseconds() % 3000 ) / 3000.f;
		//float v = std::sin(k  * M_PI * 2) * 30 + 60;
	}

	return 0;
}