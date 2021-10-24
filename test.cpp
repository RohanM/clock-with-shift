#include <iostream>
#include <assert.h>
#include "testing.h"
#include "clockwithshift.h"

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
  //std::cout << "\n";

  return getDigitalOutput(output_pin);
}

int* record_loop(int gate[], int output_pin, long timestep, int num_steps) {
     int* output = new int[num_steps];

     for(int i=0; i < num_steps; i++) {
       output[i] = run_loop(gate[i], output_pin, timestep);
       //std::cout << output[i] << "\n";
     }

     return output;
}

void compare_output(int gates[], int output[], int expected[], int length) {
  bool failed = false;

  for (int i=0; i<length; i++) {
    std::cout << gates[i] << " -> " << output[i] << " (" << expected[i] << ")\n";
    //assert(output[i] == expected[i]);
    failed = failed || output[i] != expected[i];
  }

  std::cout << (failed ? "✗" : "✓") << "\n";
}

void test_loop_running() {
  setAnalogInput(UPPER_POT, 499);
  setAnalogInput(MIDDLE_POT, 250);
  setAnalogInput(LOWER_POT, 0);

  setup();

  for(int i=0; i<10; i++) {
    //std::cout << "\n\n";
    run_loop(1, 100, 0);
    //std::cout << "\n\n";
    run_loop(0, 100, 0);
    //std::cout << "\n\n";
    run_loop(1, 100, 0);
  }
}

void test_noop() {
  // Given we have mult and div set to 1
  // And we're on simple mode and beatshift is disabled
  initialise(0, 0, 0, 0);

  // When I run the loop and record the output
  int gates[] = {0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1};
  int expected[] = {0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0};

  int* output = record_loop(gates, UNSHIFTED_OUT, 100, 15);

  // Then I should see output pulses matching the input
  compare_output(gates, output, expected, sizeof(gates) / sizeof(*gates));
}

void test_mult() {
  // Given we have mult set to 2, div set to 1
  // And we're on simple mode and beatshift is disabled
  initialise(167, 0, 0, 0);

  // When I run the loop and record the output
  int gates[] = {0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1};
  int expected[] = {1, 0, 1, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1};

  int* output = record_loop(gates, UNSHIFTED_OUT, 100, 15);

  // Then I should see output pulses matching the expected values
  compare_output(gates, output, expected, sizeof(gates) / sizeof(*gates));
}

void test_div() {
  // Given we have mult set to 1, div set to 2
  // And we're on simple mode and beatshift is disabled
  initialise(0, 167, 0, 0);

  // When I run the loop and record the output
  int gates[] = {0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1};
  int expected[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0};

  int* output = record_loop(gates, UNSHIFTED_OUT, 100, 30);

  // Then I should see output pulses matching the expected values
  compare_output(gates, output, expected, sizeof(gates) / sizeof(*gates));
}

void test_beatshift() {
  // Given we have mult set to 2, div set to 1
  // And we're on simple mode and beatshift is at 50%
  initialise(167, 0, 0, 512);

  // When I run the loop and record the output
  int gates[] = {0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1};
  int expected[] = {0, 0, 1, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0};

  int* output = record_loop(gates, SHIFTED_OUT, 100, 15);

  // Then I should see output pulses matching the expected values
  compare_output(gates, output, expected, sizeof(gates) / sizeof(*gates));
}

void test_beatshift_unshifted() {
  // Given we have mult set to 2, div set to 1
  // And we're on simple mode and beatshift is at 50%
  initialise(167, 0, 0, 512);

  // When I run the loop and record the *unshifted* output
  int gates[] = {0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1};
  int expected[] = {0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1};

  int* output = record_loop(gates, UNSHIFTED_OUT, 100, 15);

  // Then I should see unshifted beats
  compare_output(gates, output, expected, sizeof(gates) / sizeof(*gates));
}

void test_trigger_length() {
  // Given we have mult set to 2, div set to 1
  // And we're on simple mode and beatshift is disabled
  initialise(167, 0, 0, 0);

  // When I run the loop in 4ms blocks
  int gates[] = {0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
  int expected[] = {1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0};

  int* output = record_loop(gates, UNSHIFTED_OUT, 4, 15);

  // Then I should see output pulses for a 20ms duration
  compare_output(gates, output, expected, sizeof(gates) / sizeof(*gates));
}


int main() {
  std::cout << "\n\ntest_loop_running()\n";
  test_loop_running();

  std::cout << "\n\ntest_noop()\n";
  test_noop();

  std::cout << "\n\ntest_mult()\n";
  test_mult();

  std::cout << "\n\ntest_div()\n";
  test_div();

  std::cout << "\n\ntest_beatshift()\n";
  test_beatshift();

  std::cout << "\n\ntest_beatshift_unshifted()\n";
  test_beatshift_unshifted();

  std::cout << "\n\ntest_trigger_length()\n";
  test_trigger_length();

  return 0;
}
