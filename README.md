# Cube4Fun_v2
![alt text][logo]

Version 2 with ESP8266 instead of Arduino/Network

## Index
 1. [Overview](#overview)
 2. [Requirements](#requirements)
 3. [Software](#software)
 4. [Hardware](#hardware)

## Overview
* Create your own cool 3D animations and tag them
* Upload to your Cube4Fun
* Use the animation-tag from any internet service/device by sending GET/  request to activate animations
* Share your animations with everybody on [this site:](http://www.cube4fun.net)

![alt text][overviewIMG1]

## Requirements

## Software
### Cube
The communication is working through I2C protocoll. The basic logic is very simple:
* Display programmed smooth animations
* If a frame (full 64 led values) received, display it
* 5 seconds timeout for the default state. (Currently displaying programmed animations) 

#### Protocoll
* Only listen, no call backs
* valid format for a frame: 


| Key       | Position:"Value"         |
| ---       | ---                      |
| Start key | 0:"/"<br>1:"/"<br>2:"?"<br>3:"?" |
| Payload   | 4-68:"0...255"           |
| End key   | 69:","<br>70:","<br>71:" "<br>72:" " |

## Hardware
Because most of the esp8266 boards are working with 3.3V and our Cube board is a regular ATMEGA328 with 5V, we need a voltage shifter. This is a basic schematic realized with a BSS138 MOSFET. Breakout boards are available on most popular sites. 

![alt_text][schematic1] 


[logo]: http://cube4fun.net/images/Cube6-128j.png "Logo"
[overviewIMG1]: http://cube4fun.net/images/Overview-Pic2.png "Overview"
[schematic1]: https://github.com/workinghard/Cube4Fun_v2/blob/master/img/Cube4Fun_Schematic.png "Schematic1"
