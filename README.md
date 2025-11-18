# Xbox-Sound-Light-board
An add-on board for the Phat Xbox 360, featuring sound and lights at boot.

This project started when my father got a second Xbox 360 and wanted me to mod it so he could back up 
up his collection, and the grandkids could play and not destroy his collection, plus the added benefit of 
A system linking with only one copy of a game was born from all that was done. Now I would like to add an ESP32 
so I can add  custom boot sounds like clips from game themes and custom LED sequences with each boot sound, 
Sooner or later, I would love to add the capability to control the LEDs and test boot sounds from the web interface
But for now, I hope to get something to function and maybe meet some people along the way who would like to 
Help out with a little winter project.



 * Xbox 360 Boot Sound Player (Internal DAC Version)
 *
 * This version uses the ESP32's built-in 8-bit DAC on GPIO 25.
 * Using a WAV file (instead of MP3) significantly reduces CPU load,
 * resulting in smoother, more reliable audio playback.
 *
 * * Hardware:
 * - ESP32
 * - 1kOhm and 2kOhm resistors (for voltage divider)
 * - Small Speaker
 * - Spare USB Cable
 * - OPTIONAL: LM386 Amp or Powered Speakers
 *
 * * Wiring:
 * * ESP32 Power (from Xbox USB):
 * - Xbox USB +5V (Red)   -> ESP32 VIN
 * - Xbox USB GND (Black)  -> ESP32 GND
 * * Voltage Divider (Sense Pin):
 * - (This logic is removed for this test version)
 * * Audio Output (DAC):
 * - ESP32 GPIO 25 -> Audio Out + (to speaker/amp input)
 * - ESP32 GND     -> Audio Out - (to speaker/amp ground)
 */
