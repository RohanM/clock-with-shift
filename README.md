# Alternate firmware for the ginky synthese grains eurorack module

Original code by a773 (atte.dk) and released under the GPL licence

https://github.com/attejensen/a773_grains


## Changelog

- 24-10-2021 Reworked by Rohan Mitchell for easier multi/div changes between beats
- 16-10-2021 Further changes to allow for longer gaps between incoming beats and logic to handle multi/div changes between beats for Look Mum No Computer
- 11-9-2021 Adapted by Jesse Stevens of artist duo Cake Industries for Look Mum No Computer offbeat shift needs


## Getting Started

To compile and run tests:
```
make test
```

The supplied version of `clockwithshift.cpp` is prepared for compiling with g++ and running with the test
harness. To prepare for upload to an Arduino:

1. Copy `clockwithshift.cpp` to `clockwithshift.ino`
2. Comment out the existing `#include` lines
3. Uncomment the `avr` `#include` lines

This file should now compile and upload to the rack module.
