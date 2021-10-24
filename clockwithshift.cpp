/*
Alternate firmware for the ginky synthese grains eurorack module
Code by a773 (atte.dk) and released under the GPL licence
*/

/* 11-9-2021 Adapted by Jesse Stevens of artist duo Cake Industries for Look Mum No Computer offbeat shift needs*/
/* 16-10-2021 Further changes to allow for longer gaps between incoming beats and logic to handle multi/div changes between beats for Look Mum No Computer */

/*
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
*/

#include <math.h>
#include "testing.h"
#include "clockwithshift.h"

#define TRIGGER_LENGTH 20
#define UPPER_POT       2
#define MIDDLE_POT      1
#define LOWER_POT       0
#define CLOCK_IN        3
#define OFFBEAT_IN      5
#define UPPER_POT_MAX   500
#define MIDDLE_POT_MAX   500
#define LOWER_POT_MAX   500
#define NB_POT_SLICES 4
#define MODE_SIMPLE 0
#define MODE_COMPLEX 1
#define SHIFTED_OUT       11
#define UNSHIFTED_OUT     10

long now = 0;


///////////////////////////////////////////////////////////////////////////////////////////////////////////
//variables for reading the input knobs faster than at each beat
//last time we read the knobs: (we need to only read them so often or the loop will be too slow)
unsigned long lastknobread = 0;
unsigned long knobreadinginterval = 500;
///////////////////////////////////////////////////////////////////////////////////////////////////////////


/**
 * Return the duration for an output trigger to remain high.
 * This defaults to TRIGGER_LENGTH, but will reduce if our frequency is sufficiently high.
 */
/*
int trigger_length() {
  return min(TRIGGER_LENGTH, time_between_outs / 2);
}
*/


/**
 * Abstract base class for LiveControls and UnshiftedControls.
 */
class Controls {
public:
  virtual int get_mult() = 0;
  virtual int get_div() = 0;
  virtual float get_beatshift() = 0;
};


class LiveControls: public Controls {
private:
  const int SIMPLE_FACTORS[10] = {1,2,4,8,16,32,64,128,256,512};
  const int COMPLEX_FACTORS[10] = {1,3,5,7,11,13,17,19,23,29};

public:
  int mult_reading;
  int div_reading;
  int mode_reading;
  float beatshift;

  int mode;
  bool stopped;


  LiveControls() {
    mode = -1;
    stopped = false;
  }

  void setup() {
    pinMode(CLOCK_IN, INPUT_PULLUP);
    pinMode(SHIFTED_OUT, OUTPUT);
    pinMode(UNSHIFTED_OUT, OUTPUT);
  }

  void update() {
    mult_reading = analogRead(UPPER_POT);
    div_reading = analogRead(MIDDLE_POT);
    mode_reading = analogRead(LOWER_POT);
    beatshift = analogRead(OFFBEAT_IN);
  }

  void updateSettings(bool edge) {
    if (mode_reading < LOWER_POT_MAX/3) {
      // CCW simple mode
      if(edge) {
        mode = MODE_SIMPLE;
        stopped = false;
      }
    } else if(mode_reading > LOWER_POT_MAX/3*2) {
      // CW complex mode
      if(edge) {
        mode = MODE_COMPLEX;
        stopped = false;
      }
    } else {
      // stopped
      stopped = true;
    }
  }

  /**
   * Fetch the multiplication factor
   * Output clock frequency = input clock frequency * multiplication factor
   * This value is controlled by the upper pot. We divide the pot's range
   * into NB_POT_SLICES slices, and then use those to look up a factor from
   * either simple_factors (powers of two) or complex_factors (prime numbers).
   */
  int get_mult() {
    int slice = mult_reading * (NB_POT_SLICES-1) / UPPER_POT_MAX;
    return slice2factor(slice, mode);
  }

  /**
   * Fetch the division factor
   * Output clock frequency = input clock frequency / division factor
   * This value is controlled by the middle pot. We divide the pot's range
   * into NB_POT_SLICES slices, and then use those to look up a factor from
   * either simple_factors (powers of two) or complex_factors (prime numbers).
   */
  int get_div() {
    int slice = div_reading * (NB_POT_SLICES-1) / MIDDLE_POT_MAX;
    return slice2factor(slice, mode);
  }

  /**
   * Fetch the beatshift factor (scaled 0-1)
   * This value is controlled by the lower pot. Of the 0-1023 range, we introduce
   * a dead zone from 0-30, and then linearly interpolate from 0-1 for the
   * remainder of the range.
   */
  float get_beatshift() {
    if (beatshift < 30) {
      return 0;
    } else {
      return map(beatshift, 30, 1023, 0, 100) / 100.0;
    }
  }

private:
  int slice2factor(int slice, int mode) {
    if(mode == MODE_SIMPLE) {
      return SIMPLE_FACTORS[slice];
    } else {
      return COMPLEX_FACTORS[slice];
    }
  }
};


/**
 * Proxy for Controls that always returns beatshift of zero.
 */
class UnshiftedControls: public Controls {
private:
  Controls* controls;

public:
  UnshiftedControls(Controls* controls) {
    this->controls = controls;
  }

  int get_mult() {
    return controls->get_mult();
  }

  int get_div() {
    return controls->get_div();
  }

  float get_beatshift() {
    return 0;
  }
};


/**
 * GateReader reads the clock pin, detects edges and keeps time between gates.
 */
class GateReader {
private:
  bool clock_high;

public:
  GateReader() {
    clock_high = false;
  }

  bool readEdge() {
    long now = millis();
    int gate = digitalRead(CLOCK_IN);

    bool edge = false;

    // My setup is reverse logic trigger (using NPN transistor as buffer on input)
    if (gate == LOW && !clock_high) {
      edge = true;
    }
    clock_high = gate == LOW;

    return edge;
  }
};


class TimeKeeper {
private:
  Controls* controls;
  int edge_count;
  long last_edge;
  int wavelength;

  bool was_in_output_pulse;
  bool fire_trigger;
  long last_trigger;

public:
  TimeKeeper(Controls* controls) {
    this->controls = controls;
    edge_count = 0;
    last_edge = 0;
    wavelength = 0;

    was_in_output_pulse = false;
    fire_trigger = false;
    last_trigger = 0;
  }

  void update(bool edge) {
    long now = millis();

    if (edge) {
      edge_count = (edge_count + 1) % controls->get_div();

      if (edge_count == 0) {
        processEdge(now);
      }
    }

    if (haveWavelength() && outputEdge(now) && triggerDue(now)) {
      fire_trigger = true;
      last_trigger = now;
    } else {
      fire_trigger = false;
    }
  }

  bool fireTrigger() {
    return fire_trigger;
  }

private:
  // Accept an input clock and keep track of wavelength
  void processEdge(long now) {
    if (last_edge != 0) {
      wavelength = now - last_edge;
    }
    last_edge = now;
  }

  // Returns whether we've determined a wavelength reading
  bool haveWavelength() {
    return wavelength > 0;
  }

  /**
   * To determine when to show output pulses, we sample an imaginary output waveform
   * (a square wave at 50% duty cycle of the desired frequency), and then perform
   * edge detection. This method will return true when we detect a leading edge of this
   * imaginary output waveform.
   */
  bool outputEdge(long now) {
    if (inOutputPulse(now)) {
      if (!was_in_output_pulse) {
        was_in_output_pulse = true;
        return true;
      }
    } else {
      was_in_output_pulse = false;
    }
    return false;
  }

  /**
   * If we change the multiplication factor somewhere between clock signals, it might
   * be the case that we suddenly find ourselves crossing an output edge immediately
   * after another trigger. This would result in an undesirable double output pulse.
   * To prevent that, we also check that a full output wavelength has elasped since the
   * last output pulse (as calculated by this method).
   */
  bool triggerDue(long now) {
    return now - last_trigger >= wavelength / controls->get_mult();
  }

  /**
   * Sample an imaginary output waveform - a square wave at 50% duty cycle, with a wavelength
   * equal to input_wavelength * multiplication_factor.
   */
  bool inOutputPulse(long now) {
    float relative_time = (now - last_edge) / float(wavelength) + controls->get_beatshift();
    int scaled_time = relative_time * controls->get_mult() * 2;

    // Given a wavelength of 100ms and a multiplication factor of 2,
    // our relative fraction and modulo values will be:
    //   0 ms: 0; 0 % 2 = 0
    //  25 ms: 1; 1 % 2 = 1
    //  50 ms: 2; 2 % 2 = 0
    //  75 ms: 3; 3 % 2 = 1
    // 100 ms: 4; 4 % 2 = 0
    // In this example, this gives us a new wavelength of 50ms

    return scaled_time % 2 == 0;
  }
};


class Trigger {
private:
  int pin;
  int length;
  bool clock_high;

public:
  long last_trigger_out;

  Trigger(int pin) {
    this->pin = pin;
    length = TRIGGER_LENGTH;
    clock_high = false;
    last_trigger_out = 0;
  }

  // Fire the trigger, for length in ms
  void fire(int length) {
    digitalWrite(pin, HIGH);
    this->length = length;
    clock_high = true;
    last_trigger_out = millis();
  }

  // Update the trigger, setting pin to LOW when duration has expired
  void update(long now) {
    if( ((now - last_trigger_out) > length) && clock_high) {
      digitalWrite(pin, LOW);
      clock_high = false;
    }
  }
};



GateReader gateReader;
LiveControls controls;
UnshiftedControls unshiftedControls(&controls);
TimeKeeper unshiftedTimeKeeper(&unshiftedControls);
TimeKeeper shiftedTimeKeeper(&controls);
Trigger unshiftedTrigger(UNSHIFTED_OUT);
Trigger shiftedTrigger(SHIFTED_OUT);


void setup() {
  controls.setup();
  controls.update();
  Serial::begin(115200);
}


void loop()
{
  now = millis();
  bool edge = gateReader.readEdge();

  controls.updateSettings(edge);

  if (now - lastknobread >= knobreadinginterval) {
    controls.update();
    lastknobread = now;
  }

  // Fire unshifted trigger
  unshiftedTimeKeeper.update(edge);
  if (unshiftedTimeKeeper.fireTrigger()) {
    unshiftedTrigger.fire(TRIGGER_LENGTH);
  }

  // Fire shifted trigger
  shiftedTimeKeeper.update(edge);
  if (shiftedTimeKeeper.fireTrigger()) {
    shiftedTrigger.fire(TRIGGER_LENGTH);
  }

  // Trigger update
  unshiftedTrigger.update(now);
  shiftedTrigger.update(now);
}
