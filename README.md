# Alternate firmware for the ginky synthese grains eurorack module

Code by a773 (atte.dk) and released under the GPL licence


## Changelog

- 16-10-2021 Further changes to allow for longer gaps between incoming beats and logic to handle multi/div changes between beats for Look Mum No Computer
- 11-9-2021 Adapted by Jesse Stevens of artist duo Cake Industries for Look Mum No Computer offbeat shift needs


## Refactoring plan

- [ ] Build a test harness
- [ ] Create a simple test that shows if behaviour has changed
- [ ] Prettify
- [ ] Decompose loop() into parts
- [ ] Refactor and clean up until the bug becomes obvious
- [ ] Fix it ;)


### Test harness

- pinMode()
- analogRead()
- Serial.begin()
- Serial.print()
- Serial.println()
- millis()
- digitalRead()


### Acceptance tests

- Set mult and div to 1, check we get our pulses back out
- Modify mult, check output
- Modify div, check output
- Do the same with complex mode
- Try a couple of values for beatshift
- Verify trigger length

Test across both unshifted and shifted outputs.
