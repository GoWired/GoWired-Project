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

## 2020-10-22 Changelog
- Important changes to Heating Controller software, including improved coopeation with Home Assistant; Heating Controller software can now be used with any GetWired shield
- Thermal protection for dimmers
- PRESENTATION_DELAY to improve presentation accuracy
- Some minor improvements
- Fixed a bug causing repetitive calling of InitConfirmation() function in case when no output was configured
- Using restart button on a gateway now does not restart modules connected to it

## 2020-11-13 Changelog
- Important changes to Ethernet Gateway firmware: added AVR Watchdog and controller uplink testing; minor improvements

##
**Be careful!** This is still work in progress and can have some bugs. The code in master branch should always be usable. Dev branch may not be tested, use on your own responsibility.
