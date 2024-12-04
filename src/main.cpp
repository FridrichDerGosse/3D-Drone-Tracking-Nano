#include <Arduino.h>
#include "drivers/comms/armsom.hpp"
#include "drivers/fan.hpp"


void setup()
{
	// fan setup
	fan::setup();
	fan::off();

	// serial comms
	Serial.begin(9600);
}


void loop()
{
	if (Serial.available())
	{
		// turn fan on while receiving
		fan::full();

		// receive message
		String buffer;
		armsom::read_string(&buffer);

		// echo message
		armsom::write_string(buffer);

		delay(100);

		// turn fan back off
		fan::off();
	}
}
