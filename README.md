# ringcube

While everybody seems to use a smartphone to get up in the morning, I still prefer a dedicated alarm clock.
Since no available alarm clock matched all my requirements, I decided to build my own.

Requirements:

* Solar powered
* Long standby time (in the dark)
* Play music from SD card
* Dedicated "+8h" quickset alarm time button
* Warn before the battery runs out (the old one failed to do this)
* Radio controlled

The final clock is a 6cm cube (hence the name) full of batteries, circuitry and cables:

* Arduino-compatible ATmega328 @ 8 MHz
* DS3231 Real-Time Clock with a 3V lithium backup battery (>9 years runtime, temperature compensated = super precise!)
* DCF77 receiver with antenna that should be able to receive a time signal in all of Europe
* TPA2016 2.8W audio amplifier (+ speaker)
* 0.3 W solar cell
* 3 * 1.2V AAA battery (months of standby in the dark)
* Micro SD card socket
* 7-segment leds, buttons

This repo contains the design of the 3D-printed case, the circuit diagram and the software running the clock.

Several additional arduino libraries are required to compile the software:
* https://github.com/tfg13/DS3231 (to control the RTC)
* https://github.com/tfg13/TMRpcm (to play music from the sd card)
* https://github.com/adafruit/Adafruit-TPA2016-Library (to control the amplifier)
* https://github.com/greiman/SdFat (to access the sd card (official sd library cannot handle re-opening the card after sleep))
