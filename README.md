# Simple ESP32 remote control outlet

Features:
 - Site configuration is stored in flash and can be reconfigured via serial connection
 - Regularly sends outlet status to influxdb
 - Supports OTA updates via arduino ide and platformio
 - Polls a URL and turns on and off a device accordingly. (coming soon)

## Hardware Assumptions

You're running an ESP32. We use [this one](https://www.aliexpress.com/item/32906857429.html)

You have a powerful enough relay to flip on your "outlet". We use these:
  -  [250V 2A relay](https://www.amazon.com/gp/product/B00NKVLXY8/). Note: This relay requires 5V, which if your powering via USB can be taken from the VIN pin.
  -  [380V 25A relay](https://www.amazon.com/gp/product/B074FT4VXB)

You have some kind of feedback tellin gyou if the power if on or off:
  - [220V AC Detection board](https://www.aliexpress.com/item/32828199766.html). Note we use a 10k pull-up resistor adn the output is inverted.



## Building & Installing

I use platformIO, once installed, it should read the platformio.ini file for the rest.
You'll also need to install my fork of the arduino [ConfigTool](https://github.com/infinite-tree/ConfigTool) library. A stupid way to do this is symlink the ConfigTool.h and .cpp files into your local src directory.


## MicroPython Environment Installation

```
sudo apt get install esptool
esptool.py --port /dev/ttyUSB0 erase_flash
esptool.py --port /dev/ttyUSB0 --baud 460800 write_flash --flash_size=detect 0 ./micropython/esp32spiram-idf4-20191220-v1.12.bin
```


## Installing the Python code

```
pip3 install adafruit-ampy --upgrade
ampy --port /dev/ttyUSB0 put main.py /main.py
ampy --port /dev/ttyUSB0 put config.json /config.json
```

