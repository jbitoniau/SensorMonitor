#include "I2Cdev.h"
#include "MPU6050/MPU6050.h"

#include <iostream>
#include <vector>

//MPU6050 mpu;
MPU6050 mpu(0x69); // <-- use for AD0 high

std::vector<uint16_t> gyroFifoSizes;
std::vector<uint16_t> gyroXValues;
uint8_t fifoBuffer[4096]; // FIFO storage buffer


void setup() 
{
	gyroXValues.reserve(20000);
	
    // initialize device
    std::cout << "Initializing I2C devices..." << std::endl;
    mpu.initialize();

    // verify connection
    std::cout << "Testing device connections...";
	std::cout << mpu.testConnection() ? "MPU6050 connection successful" : "MPU6050 connection failed";
	std::cout << std::endl;
	
	mpu.setRate(6);
	
	std::cout << "rate: " << (int)mpu.getRate() << std::endl;
	
	mpu.setXGyroFIFOEnabled(false);
	mpu.resetFIFO();
	
	std::cout << "getTempFIFOEnabled: " << mpu.getTempFIFOEnabled() << std::endl;
    std::cout << "getXGyroFIFOEnabled: " << mpu.getXGyroFIFOEnabled() << std::endl;
    std::cout << "getYGyroFIFOEnabled: " << mpu.getYGyroFIFOEnabled() << std::endl;
    std::cout << "getZGyroFIFOEnabled: " << mpu.getZGyroFIFOEnabled() << std::endl;
    std::cout << "getAccelFIFOEnabled: " << mpu.getAccelFIFOEnabled() << std::endl;
    std::cout << "getSlave2FIFOEnabled: " << mpu.getSlave2FIFOEnabled() << std::endl;
    std::cout << "getSlave1FIFOEnabled: " << mpu.getSlave1FIFOEnabled() << std::endl;
    std::cout << "getSlave0FIFOEnabled: " << mpu.getSlave0FIFOEnabled() << std::endl;
        

/*	
mpu.resetFIFO();
usleep(1000);
std::cout << "!!!\n";
std::cout << "FIFOCount: " << mpu.getFIFOCount() << std::endl;
std::cout << "!!!\n";

	mpu.setXGyroFIFOEnabled(true);
        
	std::cout << "getTempFIFOEnabled: " << mpu.getTempFIFOEnabled() << std::endl;
    std::cout << "getXGyroFIFOEnabled: " << mpu.getXGyroFIFOEnabled() << std::endl;
    std::cout << "getYGyroFIFOEnabled: " << mpu.getYGyroFIFOEnabled() << std::endl;
    std::cout << "getZGyroFIFOEnabled: " << mpu.getZGyroFIFOEnabled() << std::endl;
    std::cout << "getAccelFIFOEnabled: " << mpu.getAccelFIFOEnabled() << std::endl;
    std::cout << "getSlave2FIFOEnabled: " << mpu.getSlave2FIFOEnabled() << std::endl;
    std::cout << "getSlave1FIFOEnabled: " << mpu.getSlave1FIFOEnabled() << std::endl;
    std::cout << "getSlave0FIFOEnabled: " << mpu.getSlave0FIFOEnabled() << std::endl;

usleep(1000);
std::cout << "!!!\n";
std::cout << "FIFOCount: " << mpu.getFIFOCount() << std::endl;
std::cout << "!!!\n";
*/
	mpu.setXGyroFIFOEnabled(true);
	mpu.setFIFOEnabled(true);
}


uint8_t bytes[4096];

void loop() 
{
	uint8_t mpuIntStatus = mpu.getIntStatus();
	uint16_t fifoCount = mpu.getFIFOCount();
	if ( mpuIntStatus & 0x10 ) //|| fifoCount==1024 )
	{
		mpu.resetFIFO();
		std::cout << "resetted! " <<  fifoCount << std::endl;
	} 
	else if (mpuIntStatus & 0x01) 
	{
		gyroFifoSizes.push_back( fifoCount );
		
		mpu.getFIFOBytes( bytes, fifoCount );
		
		for ( int i=0; i<fifoCount; i+=2 ) 		
		{
			uint8_t l = bytes[i];
			uint8_t h = bytes[i+1];
    		uint16_t v = l + h; // << 8) | h;
			gyroXValues.push_back(v);
		}
		
		
/*		for ( int i=0; i<fifoCount; i+=2 ) 		// /2 because 2 bytes per measure
		{
			uint8_t l = mpu.getFIFOByte();	// EXTREMELY SLOW!!!
			uint8_t h = mpu.getFIFOByte();
    		uint16_t v = l + h; // << 8) | h;
			gyroXValues.push_back(v);
		}*/
		//std::cout << ".";
	}
}

int main(int argc, char** argv)
{
	setup();
	unsigned int sleepTimeInUs = 1000;
	unsigned int numIterationInAS = 1000000 / sleepTimeInUs;
	std::cout << "sleepTimeInUs: " << numIterationInAS << std::endl;
	std::cout << "numIterationInAS: " << numIterationInAS << std::endl;
	unsigned int numSeconds = 10;
	for ( int i=0; i<numSeconds * numIterationInAS; ++i) //; ++i)
	{
		loop();
	//	usleep(sleepTimeInUs); //sleepTimeInUs/2);
		
		//if ( i%100==0 )
		//	std::cout << "." << std::endl;
	}
	
	std::cout << "NUM VALUES=" << gyroXValues.size() << std::endl;
	for ( int i=0; i<500; ++i )
	{
		usleep(100);
		std::cout << gyroXValues[i] << std::endl;
	}
	/*std::cout << "NUM VALUES=" << gyroFifoSizes.size() << std::endl;
	for ( int i=0; i<100; ++i )
	{
		std::cout << gyroFifoSizes[i] << std::endl;
	}*/
	
	return 0;
}


