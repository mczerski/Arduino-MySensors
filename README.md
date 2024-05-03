# Arduino-MySensors

Arduino sketchbook for [MySensorsToolkit](https://github.com/mczerski/MySensorsToolkit) example skeches.
```
git clone https://github.com/mczerski/Arduino-MySensors
git submodule update --init
```
and then setup Arduino IDE to point to this folder as a sketchbook.

Hardware setup:
- install MySensors AVR boards from https://github.com/mysensors/ArduinoBoards
- select Sensebender board
- select USBASP as pogrammer
- program sketch with programmer

FOTA:
- program sketch using Arduino IDE
- upload hex over-the-air
- program bootloader using Arduino IDE
- board should load uploaded sketch
