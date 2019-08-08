#!/bin/sh
esptool.py --chip esp32 --port /dev/ttyUSB0 --baud 921600 write_flash  0x111000 img.ffat
