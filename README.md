# Simple ESP32 remote control outlet

Features:
 - Site configuration is stored in json. Just copy a new file over the serial connection.
 - Regularly sends outlet status to influxdb
 - Polls a URL and turns on and off a device accordingly.

## Hardware Assumptions

You're running an ESP32. We use [this WROVER one](https://www.aliexpress.com/item/4000064597840.html?spm=a2g0s.9042311.0.0.70504c4dpiaF4W)

You have a powerful enough relay to flip on your "outlet". We use these:
  -  [250V 2A relay](https://www.amazon.com/gp/product/B00NKVLXY8/). Note: This relay requires 5V, which if your powering via USB can be taken from the VIN pin.
  -  [380V 25A relay](https://www.amazon.com/gp/product/B074FT4VXB)

You have some kind of feedback telling you if the power if on or off:
  - [220V AC Detection board](https://www.aliexpress.com/item/32828199766.html). Note we use a 10k pull-up resistor and the output is inverted. 



## Building & Installing


### MicroPython Environment Installation

Download [micropython (spiram-idf4) here](https://micropython.org/download/esp32/)

```
sudo apt get install esptool
esptool.py --port /dev/ttyUSB0 erase_flash
esptool.py --port /dev/ttyUSB0 --baud 460800 write_flash --flash_size=detect 0x1000 ./micro-python/esp32spiram-idf4-20191220-v1.12.bin
```


### Installing the Python code

```
pip3 install adafruit-ampy --upgrade
ampy --port /dev/ttyUSB0 put config.py /config.py
ampy --port /dev/ttyUSB0 put main.py /main.py
```

