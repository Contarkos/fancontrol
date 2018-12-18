# fancontrol

This project aims at controlling a standard 4-PIN fan based on the surrounding temperature. It mainly relies on the GPIOs of the Raspberry Pi B+.

The main goal here is for me to discover the joy of low-level programming on an embedded ARM architecture.

## Control the fan's speed

* Use of the PWM pins on the Raspberry Pi to control the 4-PIN fans
* Separation between command and power circuits.

## Measure the temperature

* Configure an AD7705 ADC through SPI
* Measure the temperature using a thermistor

## Drive the software via interrupts

* Use of custom drivers listening on GPIOs state to deliver interrupts to run the software
