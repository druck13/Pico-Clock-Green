Pico-Clock-Green
================

This repo contains an improved version of the demo software for the Raspberry Pi Pico based clock.

It is supplied under the BSD-3-clause license, as the original on the WaveShare website.

Please see https://www.waveshare.com/wiki/Pico-Clock-Green for full details.

I have no affiliation with WaveShare, and cannot offer support for the clock or any other products.

New Features
------------
* A 3rd level of dimming in very low light levels - this may allow you to sleep with the clock next to the bed!
* An improved number 7, instead of the horrible default hook shape.

Code Changes
------------
* A cmake file which actually works on the Raspberry Pi, follow the instructions at the link above on what to install and what commands to run.
* All comments in Chinese translated in to English using Google translate, but they do make sense!
* Added a 3rd dimming lower level, increased refresh timer rate from 150us to 50us to prevent LED illumination. 
* Font data changed from hex to binary constants to make it easier to edit, although the binary shows the characters as reversed.

Tip
---
The clock can be reprogrammed by connecting the external USB socket to a computer, but you have to have the back off in order to press the programming button on the Pico, which is inconvenient if you want to keep experimenting with the software. If you drill a 3mm hole, 82.5mm from the left and 35mm from the bottom from of the outer face of the back cover, you can keep the back on and use a pen to press the botton.

Future Enhancements
-------------------
* Move the C/F temperature unit selection to the top button set-up sequence.
* Use the middle button short press to toggle between time, day/month (or month/day if set to F) and temperature, returning to time after 10 seconds.
* Think of a button combination to also allow display of light level and voltage.
* Add DST rules.
