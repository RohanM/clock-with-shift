#include <iostream>
#include <assert.h>
#include "testing.h"
#include "clockwithshift.h"

void test_setup_initial_values() {
  // Given some values for the pots
  setAnalogValue(UPPER_POT, 3);
  setAnalogValue(MIDDLE_POT, 2);
  setAnalogValue(LOWER_POT, 1);

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

void run_loop(int gate, long duration) {
    setDigitalValue(CLOCK_IN, gate);
    loop();
    advanceTime(duration);
}

void test_loop_running() {
  setAnalogValue(UPPER_POT, 499);
  setAnalogValue(MIDDLE_POT, 250);
  setAnalogValue(LOWER_POT, 0);

  setup();

  for(int i=0; i<10; i++) {
    std::cout << "\n\n";
    run_loop(1, 100);
    std::cout << "\n\n";
    run_loop(0, 100);
    std::cout << "\n\n";
    run_loop(1, 100);
  }
}

int main() {
  std::cout << "test_setup_initial_values()\n";
  test_setup_initial_values();

  std::cout << "\n\ntest_loop_running()\n";
  test_loop_running();

  return 0;
}
