# ArduinoJSN-SR04T-V2
Arduino project to connect 4 distance sensors to an arduino mini and communicate the measure with SPI 
to a Raspberry Pi

## JSN SR04T V2.0 are in Mode 2

For this was needed 4 x 47K resistor (one for each modul).
See about Mode 2 in the file docs/JSN-SR04T-2.0.pdf

## Overview

Notes:
* The four distance sensors is JSN-SR04T-V2 and not HC-SR04 as in immage (didn't found any in fritzing)
* Level shifter is because the RPI is 3.3V and the used Arduino Mini Pro is 5V
* The RPI (master) Arduino (slave) communicate with SPI (used a very simple 4 bytes transfer)

![schema docs] (https://github.com/iszilagyigit/ArduinoJSN-SR04T-V2/blob/master/fritzing/parkdist-sensor_bb.png)
<img align="left" width="400" height="200" src="https://github.com/iszilagyigit/ArduinoJSN-SR04T-V2/blob/master/fritzing/parkdist-sensor_bb.png">

