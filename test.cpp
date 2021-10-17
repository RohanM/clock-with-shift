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

int main() {
  test_setup_initial_values();

  return 0;
}
