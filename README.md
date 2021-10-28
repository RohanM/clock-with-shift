# Clock multiplier / divider / beatshift

The [original code by a773](https://github.com/attejensen/a773_grains) (atte.dk) (released under
the GPL licence) provided alternate firmware for the ginky synthese grains eurorack module.

This has now been extended to provide a general purpose clock multipler / divider / beatshift
code for use as a synthesiser module.


## Hardware

This module runs on an Arduino (tested on an Arduino Nano). It expects four potentiometers as controls
(pins defined by `MULT_POT`, `DIV_POT`, `MODE_POT` and `BEATSHIFT_POT`). It takes a clock as input
on the `CLOCK_IN` pin and has two trigger outputs - `SHIFTED_OUT` and `UNSHIFTED_OUT` (the latter
identical but without the beatshift applied).


## Operation

The module can operate in two modes:

- Simple mode: Multiplies or divides by factors of two (1, 2, 4, 8, 16, ...)
- Complex mode: Multiplies or divides by prime numbers (1, 3, 5, 7, 11, ...)

The mode can be selected by moving the mode knob to the left or right side. A range of output
tempos can be produced by combining multiplication and division, especially by using complex factors.
The beatshift knob can be used to delay the trigger from the beatshifted output by up to one beat.


## Changelog

- 24-10-2021 Reworked by Rohan Mitchell for easier multi/div changes between beats
- 16-10-2021 Further changes to allow for longer gaps between incoming beats and logic to handle multi/div changes between beats for Look Mum No Computer
- 11-9-2021 Adapted by Jesse Stevens of artist duo Cake Industries for Look Mum No Computer offbeat shift needs
