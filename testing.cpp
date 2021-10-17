#include <iostream>
#include "testing.h"

void pinMode(int pin, int mode) {
  std::cout << "pinMode(" << pin << ", " << mode << ")\n";
}

int analogRead(int pin) {
  std::cout << "analogRead(" << pin << ")\n";

  return 0;
}

int digitalRead(int pin) {
  std::cout << "digitalRead(" << pin << ")\n";

  return 0;
}

int digitalWrite(int pin, int value) {
  std::cout << "digitalWrite(" << pin << ", " << value << ")\n";

  return 0;
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

void Serial::println(char* str) {
  std::cout << "Serial.println(" << str << ")\n";
}
