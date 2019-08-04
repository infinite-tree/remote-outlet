# Simple ESP32 remote control outlet

Features:
 - Site configuration is stored in flash and can be reconfigured via serial connection
 - Regularly sends outlet status to influxdb
 - Supports OTA updates via arduino ide and platformio
 - Polls a URL and turns on and off a device accordingly. (coming soon)

## Hardware Assumptions

You're running an ESP32. We use [this one](https://www.aliexpress.com/item/32906857429.html)

You have a powerful enough relay to flip on your "outlet". We use these:
  -  [250V 2A relay](https://www.amazon.com/gp/product/B00NKVLXY8/)
  -  [380V 25A relay](https://www.amazon.com/gp/product/B074FT4VXB)

You have some kind of feedback tellin gyou if the power if on or off:
  - [220V AC Detection board](https://www.aliexpress.com/item/32828199766.html)



## Building & Installing

I use platformIO, once installed, it should read the platformio.ini file for the rest.
You'll also need to install my fork of the arduino [ConfigTool](https://github.com/infinite-tree/ConfigTool) library. A stupid way to do this is symlink the ConfigTool.h and .cpp files into your local src directory.

