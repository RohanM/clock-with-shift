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
