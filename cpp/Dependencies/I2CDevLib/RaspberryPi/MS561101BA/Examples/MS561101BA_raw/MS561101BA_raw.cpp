#include "I2Cdev.h"
#include "MS561101BA/MS561101BA.h"

MS561101BA barometer(MS561101BA_ADDRESS_CSB_LOW);
 
void setup()
{
	printf("Initializing I2C devices...");
	barometer.initialize();
	printf("Testing device connections...");
	printf(barometer.testConnection() ? "MS561101BA connection successful" : "MS561101BA connection failed");
	printf("\n");
}

void loop() 
{
	float pressure = 0.f;
	float temperature = 0.f;
	bool ret = barometer.readValues( &pressure, &temperature );
	if ( ret )
		printf("Pressure: %fhPa\tTemperature: %f°C\n", pressure, temperature);
	else
		printf("Failed to read values\n");
}

int main(int argc, char** argv)
{
	setup();
	for (;;)
	{
		loop();
		usleep(100000);
	}
	return 0;
}


