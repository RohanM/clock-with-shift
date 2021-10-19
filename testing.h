// Pin types
#define INPUT_PULLUP 0
#define OUTPUT 1

// Digital pin values
#define LOW 0
#define HIGH 1


void advanceTime(long millis);
void setAnalogInput(int pin, int value);
void setDigitalInput(int pin, int value);
int getDigitalOutput(int pin);


long map(long x, long in_min, long in_max, long out_min, long out_max);
int min(int a, int b);
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
