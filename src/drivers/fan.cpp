#include "fan.hpp"

void fan::setup()
{
    pinMode(FAN_CORRECTION_PIN, INPUT);
    pinMode(FAN_PWM_PIN, OUTPUT);

    analogWrite(FAN_PWM_PIN, 80);
}

void fan::set_speed(uint8_t speed)
{
    analogWrite(FAN_PWM_PIN, speed);
}

void fan::full()
{
    digitalWrite(FAN_PWM_PIN, HIGH);
}

void fan::off()
{
    digitalWrite(FAN_PWM_PIN, LOW);
}