/*
Alternate firmware for the ginky synthese grains eurorack module
Code by a773 (atte.dk) and released under the GPL licence
*/

/* 11-9-2021 Adapted by Jesse Stevens of artist duo Cake Industries for Look Mum No Computer offbeat shift needs */
/* 16-10-2021 Further changes to allow for longer gaps between incoming beats and logic to handle multi/div changes between beats for Look Mum No Computer */
/* 24-10-2021 Reworked by Rohan Mitchell for easier multi/div changes between beats */


#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>


#define TRIGGER_LENGTH 20
#define UPPER_POT       2
#define MIDDLE_POT      1
#define LOWER_POT       0
#define CLOCK_IN        3
#define OFFBEAT_IN      5
#define UPPER_POT_MAX   1024
#define MIDDLE_POT_MAX  1024
#define LOWER_POT_MAX   1024
#define NB_POT_SLICES   10
#define MODE_SIMPLE     0
#define MODE_COMPLEX    1
#define SHIFTED_OUT     11
#define UNSHIFTED_OUT   10
#define KNOB_READING_INTERVAL 500


/**
 * Interface for reading and interpreting the control knobs.
 */
class Controls {
private:
  const int SIMPLE_FACTORS[10] = {1, 2, 4, 8, 16, 32, 64, 128, 256, 512};
  const int COMPLEX_FACTORS[10] = {1, 3, 5, 7, 11, 13, 17, 19, 23, 29};

public:
  int mult_reading;
  int div_reading;
  int mode_reading;
  float beatshift;

  int mode;
  bool stopped;


  Controls() {
    mode = -1;
    stopped = false;
  }

  void setup() {
    pinMode(CLOCK_IN, INPUT_PULLUP);
    pinMode(SHIFTED_OUT, OUTPUT);
    pinMode(UNSHIFTED_OUT, OUTPUT);
  }

  void read() {
    mult_reading = analogRead(UPPER_POT);
    div_reading = analogRead(MIDDLE_POT);
    mode_reading = analogRead(LOWER_POT);
    beatshift = analogRead(OFFBEAT_IN);

    Serial.print(get_mult());
    Serial.print(", ");
    Serial.print(get_div());
    Serial.print(", ");
    Serial.println(get_beatshift());
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
    int slice = mult_reading * (NB_POT_SLICES) / UPPER_POT_MAX;
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
    int slice = div_reading * (NB_POT_SLICES) / MIDDLE_POT_MAX;
    int factor = slice2factor(slice, mode);

    return factor;
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
      return map(beatshift, 30, 1023, 0, 95) / 100.0;
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
 * GateReader reads the clock pin and detects edges.
 */
class GateReader {
private:
  bool clock_high;

public:
  GateReader() {
    clock_high = false;
  }

  bool readEdge(long now) {
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


/**
 * Given control inputs and fed input edges, TimeKeeper will report the output
 * wavelength and when to fire an output trigger (taking mult and div factors
 * into account).
 */
class TimeKeeper {
private:
  Controls* controls;
  int edge_count;
  long last_edge;
  long last_phrase_start;
  long wavelength;

  bool was_in_output_pulse;
  bool fire_trigger;
  long last_trigger;

public:
  TimeKeeper(Controls* controls) {
    this->controls = controls;
    edge_count = 0;
    last_edge = 0;
    last_phrase_start = 0;
    wavelength = 0;

    was_in_output_pulse = false;
    fire_trigger = false;
    last_trigger = 0;
  }

  void update(long now, bool edge) {
    if (edge) {
      processEdge(now);
    }

    bool outputEdge = this->outputEdge(now);

    // Detect start of phrase
    if (edge && now > readyForEndOfPhrase()) {
      last_phrase_start = now;
    }

    fire_trigger = false;
    if (haveWavelength() && outputEdge) {
      fire_trigger = true;
      last_trigger = now;
    }
  }

  long outputWavelength() {
    return float(wavelength) / controls->get_mult();
  }

  // Returns whether to fire the output trigger
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

  /**
   * To determine when to show output pulses, we sample an imaginary output waveform
   * (a square wave at 50% duty cycle of the desired frequency), and then perform
   * edge detection. This method will return true when we detect a leading edge of this
   * imaginary output waveform.
   */
  bool outputEdge(long now) {
    bool edge = false;
    
    if (inOutputPulse(now)) {
      if (!was_in_output_pulse) {
        was_in_output_pulse = true;
        edge = true;
      }
    } else {
      was_in_output_pulse = false;
    }
    
    return edge;
  }

  /**
   * Sample an imaginary output waveform - a square wave at 50% duty cycle, with a wavelength
   * equal to input_wavelength * multiplication_factor.
   */
  bool inOutputPulse(long now) {
    long offset = now - last_phrase_start;
    float relative_time = offset / float(wavelength);
    long scaled_time = relative_time * scaleFactor() * 2;

    // Given a wavelength of 100ms and a multiplication factor of 2,
    // our relative fraction and modulo values will be:
    //   0 ms: 0; 0 % 2 = 0
    //  25 ms: 1; 1 % 2 = 1
    //  50 ms: 2; 2 % 2 = 0
    //  75 ms: 3; 3 % 2 = 1
    // 100 ms: 4; 4 % 2 = 0
    // In this example, this gives us a new wavelength of 50ms

    if (now > last_phrase_start + phraseLength()) {
      return false;
    } else {
      return scaled_time % 2 == 0;
    }
  }

  /**
   * Return a time when we've received all our clocks for the phrase
   * and the next clock we get should be considered the start of the
   * next phrase.
   */
  long readyForEndOfPhrase() {
    long delta = wavelength / 2;
    return last_phrase_start + phraseLength() - delta;
  }

  // Returns whether we've determined a wavelength reading
  bool haveWavelength() {
    return wavelength > 0;
  }

  float scaleFactor() {
    return controls->get_mult() / float(controls->get_div());
  }

  long beatLength() {
    return wavelength / scaleFactor();
  }

  long phraseLength() {
    return long(wavelength) * controls->get_div();
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
  void fire(long now, int length) {
    digitalWrite(pin, HIGH);
    this->length = length;
    clock_high = true;
    last_trigger_out = now;
  }

  // Update the trigger, setting pin to LOW when duration has expired
  void update(long now) {
    if( ((now - last_trigger_out) > length) && clock_high) {
      digitalWrite(pin, LOW);
      clock_high = false;
    }
  }
};



long now = 0;
unsigned long last_knob_read = 0;

class TimeFollower {
private:
  Controls* controls;
  long last_signal;
  long wavelength;
  bool output_fired;

public:
  TimeFollower(Controls* controls) {
    this->controls = controls;
    last_signal = 0;
    wavelength = 0;
    output_fired = false;
  }

  bool shouldFire(long now, bool signal) {
    // Detect edges and track last signal
    if (signal) {
      if (last_signal > 0) {
        wavelength = now - last_signal;
      }
      last_signal = now;
      output_fired = false;
    }

    // Fire output
    if (now >= last_signal + delayTime() && !output_fired) {
      output_fired = true;
      return true;
    }

    return false;
  }

private:
  long delayTime() {
    return wavelength * controls->get_beatshift();
  }
};


GateReader gateReader;
Controls controls;
TimeKeeper timeKeeper(&controls);
TimeFollower timeFollower(&controls);
Trigger unshiftedTrigger(UNSHIFTED_OUT);
Trigger shiftedTrigger(SHIFTED_OUT);


void setup() {
  controls.setup();
  controls.read();
  Serial.begin(115200);
}

void loop()
{
  now = millis();
  bool edge = gateReader.readEdge(now);

  controls.updateSettings(edge);

  if (now - last_knob_read >= KNOB_READING_INTERVAL) {
    controls.read();
    last_knob_read = now;
  }

  int trigger_length = min(TRIGGER_LENGTH, timeKeeper.outputWavelength() / 2);

  // Fire unshifted trigger
  timeKeeper.update(now, edge);
  if (timeKeeper.fireTrigger()) {
    unshiftedTrigger.fire(now, trigger_length);
  }

  // Fire shifted trigger
  if (timeFollower.shouldFire(now, timeKeeper.fireTrigger())) {
    shiftedTrigger.fire(now, trigger_length);
  }

  // Trigger update
  unshiftedTrigger.update(now);
  shiftedTrigger.update(now);
}
