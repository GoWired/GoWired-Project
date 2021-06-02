![Logo](https://github.com/feanor-anglin/GetWired-Project/blob/master/Images/GetWired_small.png)

This is a repository for **GetWired**, RS485 & MySensors based, open source home automation hardware previously released at [openhardware.io](https://www.openhardware.io/user/2098#view=projects). Lately it took part in a successful **[crowdfunding campaign](https://www.crowdsupply.com/domatic/getwired)**. It is now available to buy at [Mouser](https://www2.mouser.com/Search/Refine?Keyword=getwired).

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

## 2021-05 Changelog
- Added AVR watchdog to GetWired New Firmware
- Fixed a bug in which roller shutter modules set open position of a roller shutter after reboot; now the position is recovered from EEPROM
- Minor improvements.

##
The code in master branch should always be tested & working. Dev branch may not be tested, use on your own responsibility.
