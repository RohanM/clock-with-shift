// Pin types
#define INPUT_PULLUP 0
#define OUTPUT 1

// Digital pin values
#define LOW 0
#define HIGH 1


void pinMode(int pin, int mode);
int analogRead(int pin);
int digitalRead(int pin);
long millis();


class Serial {
  static void begin(int bitrate);
  static void print(char* str);
  static void println(char* str);
};
