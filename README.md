# arduino-xmas-883-unitSynth
This is the Unit Synth version of the [arduino-xmas-883](https://github.com/nyh-workshop/arduino-xmas-883). Please refer to this project for more info how to build.

I have initially wanted to create a branch off from the `arduino-xmas-883` project. However, to stick close to the original motivation of the author (to build an equivalent modern version of that one), I have decided to create a new repo based on it with new and unique modifications.

**Note: Please create the `data` folder in the project if you need to upload the files into the LittleFS!**

This one has:
- Shuffling of MIDI files to play files at random order. It uses this [small library](https://github.com/nyh-workshop/arduino-midiJukeBoxLibrary).
- Shuffling of FastLED functions for random patterns at every start of the unit.
- Added battery operation idle mode for another [earlier Ikea Strala mod](https://github.com/nyh-workshop/ikea-strala-2023-music).
- [Unit Synth](https://docs.m5stack.com/en/unit/Unit-Synth) support.