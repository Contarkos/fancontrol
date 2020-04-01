# fancontrol

This project aims at controlling a standard 4-PIN fan based on the surrounding temperature. It mainly relies on the GPIOs of the Raspberry Pi B+.

The main goal here is for me to discover the joy of low-level programming on an embedded ARM architecture. 

I'll try to add a self-sufficient makefile to build a minimalist ramfs distro through Buildroot to to make it look like a real embedded system.

## Setup

### Toolchain requirements

This project requires the following points in order to work :
* A toolchain located in /opt/armv6-rpi-linux-gnueabihf/bin/arm-linux-gnueabihf- or the toolchain in your PATH
* A rootfs directory in /opt/armv6-rpi-linux-gnueabihf/ (lib and include)
* A kernel source code for linux 4.14 in /opt/kernel/linux-rpi-4.14.y
* You can build your own toolchain with the configuration file .config-cross in the tools/boot directory
* You may need to edit some path and some settings before compiling the toolchain.

### On your Pi

* Use the version 4.14.70 for your kernel
* Install at least tmux on your Pi
* Copy the scripts in tool/scripts on your Pi
* Add the ltmux.sh script to your init sequence

Alternatively, you can use the buildroot .config file present in the tools/boot directory
* You need to edit the path to the .config-linux file in the buildroot configuration
* No ltmux.sh script in this configuration, you need to copy the files manually
* You can mount the partitions on your SD card for persistent file storage.

## About

### Control the fan's speed

* Use of the PWM pins on the Raspberry Pi to control the 4-PIN fans
* Separation between command and power circuits.
* Reading of the actual speed of the fan through IRQ

### Measure the temperature

* Configure an AD7705 ADC through SPI
* Measure the temperature using a thermistor

### Drive the software via interrupts

* Use of custom drivers listening on GPIOs state to deliver interrupts to run the software

## Schematics

### Circuit layout

* Coming soon...

### RPi's pins layout

* Coming soon...
