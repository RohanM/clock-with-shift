#include <iostream>
#include <assert.h>
#include "testing.h"
#include "clockwithshift.h"

void test_setup_initial_values() {
  // Given some values for the pots
  setAnalogInput(UPPER_POT, 3);
  setAnalogInput(MIDDLE_POT, 2);
  setAnalogInput(LOWER_POT, 1);

  // When I run setup()
  setup();

  // Then I should see the values on upper/middle/lower reading & reset
  assert(upperreading == 3);
  assert(middlereading == 2);
  assert(lowerreading == 1);
  assert(reset == 1);

  // And time_between_outs should be initialised to zero
  assert(time_between_outs == 0);
}

void initialise(int mult, int div, int mode, int beatshift) {
  setAnalogInput(UPPER_POT, mult);
  setAnalogInput(MIDDLE_POT, div);
  setAnalogInput(LOWER_POT, mode);
  setAnalogInput(OFFBEAT_IN, beatshift);

  setup();
}

int run_loop(int gate, int output_pin, long duration) {
  setDigitalInput(CLOCK_IN, gate);
  loop();
  advanceTime(duration);
  std::cout << "\n";

  return getDigitalOutput(output_pin);
}

int* record_loop(int gate[], int output_pin, long timestep, int num_steps) {
     int* output = new int[num_steps];

     for(int i=0; i < num_steps; i++) {
       output[i] = run_loop(gate[i], output_pin, timestep);
       std::cout << output[i] << "\n";
     }

     return output;
}

void test_loop_running() {
  setAnalogInput(UPPER_POT, 499);
  setAnalogInput(MIDDLE_POT, 250);
  setAnalogInput(LOWER_POT, 0);

  setup();

  for(int i=0; i<10; i++) {
    std::cout << "\n\n";
    run_loop(1, 100, 0);
    std::cout << "\n\n";
    run_loop(0, 100, 0);
    std::cout << "\n\n";
    run_loop(1, 100, 0);
  }
}

void test_noop() {
  // Given we have mult and div set to 1
  // And we're on simple mode and beatshift is disabled
  initialise(0, 0, 0, 0);

  // When I run the loop and record the output
  int gates[] = {0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1};
  int expected_output[] = {0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0};

  int* output = record_loop(gates, UNSHIFTED_OUT, 100, 15);

  // Then I should see output pulses matching the input
  for (int i=0; i<sizeof(gates)/sizeof(*gates); i++) {
    std::cout << "(" << gates[i] << ", " << output[i] << ")\n";
    assert(output[i] == expected_output[i]);
  }
}

void test_mult() {
  // Given we have mult set to 2, div set to 1
  // And we're on simple mode and beatshift is disabled
  initialise(167, 0, 0, 0);

  // When I run the loop and record the output
  int gates[] = {0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1};
  int expected_output[] = {1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1};

  int* output = record_loop(gates, UNSHIFTED_OUT, 100, 15);

  // Then I should see output pulses matching the input
  for (int i=0; i<sizeof(gates)/sizeof(*gates); i++) {
    std::cout << "(" << gates[i] << ", " << output[i] << ")\n";
    assert(output[i] == expected_output[i]);
  }
}

int main() {
  std::cout << "test_setup_initial_values()\n";
  test_setup_initial_values();

  std::cout << "\n\ntest_loop_running()\n";
  test_loop_running();

  std::cout << "\n\ntest_noop()\n";
  test_noop();

  std::cout << "\n\ntest_mult()\n";
  test_mult();

  return 0;
}
