# pixelated

Initial commit of code for driving a P2.5 HUB75E Panel with a Pixelated ESP32 pcb (https://pixelated.cn).

Stores config in eeprom for MQTT and WIFI settings.
If no wifi saved, then will appear as an AP with name pixelated, no password.
Connect to 192.168.4.1 to config, and save.
Boot button is configured to erase eeprom to factory defaults, and forget wifi config (if pressed post boot).
 

## To Do -
Make output code into classes for standardization

eg

Class.init

Class.display ...


Add clock code

Add Snake code

Add initial code to setup MQTT, WIFI

Fix green boogies on output

Add more FX


## Working - 

MQTT subscription / control

Plasma

GIF Animation (using FFAT)

Basic menu

Conway's Game of Life 

EEPROM config / Wifi Config

## Notes - 

FFAT upload (1M Code / 3M FFAT Partition)
```
esptool.py --chip esp32  --baud 921600 write_flash  0x111000 img.ffat

Make the img.ffat with this - https://github.com/TobleMiner/mk_esp32fat or https://github.com/marcmerlin/esp32_fatfsimage

Install esptool from here - https://github.com/espressif/esptool  or brew install esptool (on mac with https://brew.sh)
```

Requires https://github.com/mrfaptastic/ESP32-RGB64x32MatrixPanel-I2S-DMA  
(amend #define MATRIX_HEIGHT 64  in https://github.com/mrfaptastic/ESP32-RGB64x32MatrixPanel-I2S-DMA/blob/master/ESP32-RGB64x32MatrixPanel-I2S-DMA.h)
