![Logo](https://github.com/feanor-anglin/GetWired-Project/blob/master/Images/GetWired_small.png)

This is a repository for **GetWired**, RS485 & MySensors based, open source home automation hardware previously released at [openhardware.io](https://www.openhardware.io/user/2098#view=projects). Lately it took part in a successful **crowdfunding campaign**. It is now available to buy at **[Crowd Supply](https://www.crowdsupply.com/domatic/getwired)**.

## Features
- MySensors library for communication through RS485 protocol,
- FOTW updates,
- Thermal and electrical security procedures,
- Easy configuration,
- Operating with ON/OFF receivers, roller shutters, dimmable LED tapes (white, RGB, RGBW),
- Buttons: long press and short press logic,
- Sensors: built-in current sensor and temperature sensor handling.

## Wiki
To learn more, have a look at our [wiki](https://github.com/feanor-anglin/GetWired-Project/wiki).

## 2020-10-12 Changelog
- Important changes to Heating Controller software, including improved coopeation with Home Assistant; Heating Controller software can now be used with any GetWired shield
- Thermal protection for dimmers
- PRESENTATION_DELAY to improve presentation accuracy
- Some minor improvements

## 2020-08-27 Changelog:
- New feature: controlling dimmers from buttons
- Fixed issue with reading digital inputs (INPUT_1 - INPUT_4)

## 2020-08-12 Changelog:
- Support for Home Assistant
- DC current measurement
- Improved analog sensors accuracy
- New, more advanced procedure of updating current values to the controller
- Support for defining cos(fi) value of the load for current sensor
- New "Quick config" section in Configuration.h
- Roller shutter autocalibration
- SHT30 temp&hum sensor support

##
**Be careful!** This is still work in progress and can have some bugs. The code in master branch should always be usable. Developement branch may not be tested, use on your own responsibility.
