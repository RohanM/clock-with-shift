// Pin types
#define INPUT_PULLUP 0
#define OUTPUT 1

// Digital pin values
#define LOW 0
#define HIGH 1


void advanceTime(long millis);
void setAnalogValue(int pin, int value);
void setDigitalValue(int pin, int value);


long map(long x, long in_min, long in_max, long out_min, long out_max);
void pinMode(int pin, int mode);
int analogRead(int pin);
int digitalRead(int pin);
void digitalWrite(int pin, int value);
long millis();


class Serial {
 public:
  static void begin(int bitrate);
  static void print(char* str);
  static void print(int input);
  static void println(char* str);
  static void println(int input);
};
