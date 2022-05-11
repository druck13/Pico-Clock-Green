Pico-Clock-Green
================

This repo contains an improved version of the demo software for the Raspberry Pi Pico based clock.

It is supplied under the BSD-3-clause license, as the original on the WaveShare website.

Please see https://www.waveshare.com/wiki/Pico-Clock-Green for full details.

I have no affiliation with WaveShare, and cannot offer support for the clock on any other products.

New Features
------------
* A 3rd level of dimming in very low light levels - this may allow you to sleep with the clock next to the bed!
* An improved the look of the 4 and 7 digits

Code Changes
------------
* A cmake file which actually works on the Raspberry Pi, follow the instructions at the link about on what to install and what commands to run.
* All comments in Chinese translated in to English using Google translate, but they do make sense!
* Added a 3rd dimming lower level, increased refresh timer rate from 150us to 50us to prevent LED illumination. 
* Font data changed from hex to binary constants to make it easier to edit, although the binary shows the characters as reversed

Future Enhancements
-------------------
* Move the C/F temperature unit selection to the top button set-up sequence.
* Use the middle button short press to toggle between time, day/month (or month/day if set to F) and temperature, returning to time after 10 seconds.
* Think of a button combination to also allow display of light level and voltage.
