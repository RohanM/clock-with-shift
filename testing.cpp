#include <iostream>
#include "testing.h"

long time_millis = 0;
int analog_inputs[3] = {0, 0, 0};
int digital_inputs[4] = {0, 0, 0, 0};
int digital_outputs[12] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

void advanceTime(long millis) {
  time_millis += millis;
}

void setAnalogInput(int pin, int value) {
  analog_inputs[pin] = value;
}

void setDigitalInput(int pin, int value) {
  digital_inputs[pin] = value;
}

int getDigitalOutput(int pin) {
  int value = digital_outputs[pin];
  //std::cout << "getDigitalOutput(" << pin << ") -> " << value << "\n";
  return value;
}

long map(long x, long in_min, long in_max, long out_min, long out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

int min(int a, int b) {
  return a < b ? a : b;
}

void pinMode(int pin, int mode) {
  //std::cout << "pinMode(" << pin << ", " << mode << ")\n";
}

int analogRead(int pin) {
  int value = analog_inputs[pin];
  //std::cout << "analogRead(" << pin << ") -> " << value << "\n";

  return value;
}

int digitalRead(int pin) {
  int value = digital_inputs[pin];
  //std::cout << "digitalRead(" << pin << ") -> " << value << "\n";

  return value;
}

void digitalWrite(int pin, int value) {
  digital_outputs[pin] = value;
  //std::cout << "digitalWrite(" << pin << ", " << value << ")\n";
}

long millis() {
  //std::cout << "millis() -> " << time_millis << "\n";

  return time_millis;
}


void Serial::begin(int bitrate) {
  //std::cout << "Serial.begin(" << bitrate << ")\n";
}

void Serial::print(char* str) {
  std::cout << "> " << str << "\n";
}

void Serial::print(int input) {
  std::cout << "> " << input << "\n";
}

void Serial::println(char* str) {
  std::cout << "> " << str << "\n";
}

void Serial::println(int input) {
  std::cout << "> " << input << "\n";
}
