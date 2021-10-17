#include <iostream>
#include "testing.h"

int analog_values[3] = {0, 0, 0};

void setAnalogValue(int pin, int value) {
  analog_values[pin] = value;
}

long map(long x, long in_min, long in_max, long out_min, long out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void pinMode(int pin, int mode) {
  std::cout << "pinMode(" << pin << ", " << mode << ")\n";
}

int analogRead(int pin) {
  int value = analog_values[pin];
  std::cout << "analogRead(" << pin << ") -> " << value << "\n";

  return value;
}

int digitalRead(int pin) {
  std::cout << "digitalRead(" << pin << ")\n";

  return 0;
}

void digitalWrite(int pin, int value) {
  std::cout << "digitalWrite(" << pin << ", " << value << ")\n";
}

long millis() {
  std::cout << "millis()\n";

  return 0;
}


void Serial::begin(int bitrate) {
  std::cout << "Serial.begin(" << bitrate << ")\n";
}

void Serial::print(char* str) {
  std::cout << "Serial.print(" << str << ")\n";
}

void Serial::print(int input) {
  std::cout << "Serial.print(" << input << ")\n";
}

void Serial::println(char* str) {
  std::cout << "Serial.println(" << str << ")\n";
}

void Serial::println(int input) {
  std::cout << "Serial.println(" << input << ")\n";
}
