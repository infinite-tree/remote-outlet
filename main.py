import machine
from micropython import const
import network
import ubinascii
import urequests
import time

OUTLET_SENSE_PIN = const(4)
OUTLET_CONTROL_PIN = const(18)

DATA_DELAY_SEC = const(60)
CONNECTION_DELAY_SEC = const(10)

CONFIG_FILE = "config.json"
CONFIG_WIFI_SSID = "WIFI_SSID"
CONFIG_WIFI_PASSWD = "WIFI_PASSWD"

CONFIG_INFLUXDB_URL = "INFLUXDB_URL"
CONFIG_INFLUXDB_USER = "INFLUXDB_USER"
CONFIG_INFLUXDB_PASSWD = "INFLUXDB_PASSWD"

CONFIG_SENSOR_NAME = "SENSOR_NAME"
CONFIG_SENSOR_LOCATION = "SENSOR_LOCATION"

CONFIG_OUTLET_URL = "OUTLET_URL"

MEASUREMENT = "outlet_on"

WIFI = network.WLAN(network.STA_IF)
WIFI.active(True)

def loadConfig():
    print("Loading Config file")
    import config
    # there was a bug about memory needing to be in ram
    # createing anew dict from the one in flash should do it..
    return dict(config.config)


def sendDatapoint(config, value):
    connectToWifi(config)
    url = config.get(CONFIG_INFLUXDB_URL)
    auth = ubinascii.b2a_base64("{}:{}".format(config.get(CONFIG_INFLUXDB_USER), config.get(CONFIG_INFLUXDB_PASSWD)))
    headers = {
        'Content-Type': 'text/plain',
        'Authorization': 'Basic ' + auth.decode().strip()

    }
    data = "{},location={},sensor={} value={}.0".format(MEASUREMENT,
                                                        config.get(CONFIG_SENSOR_LOCATION),
                                                        config.get(CONFIG_SENSOR_NAME),
                                                        value)
    r = urequests.post(url, data=data, headers=headers)
    if len(r.text) < 1:
        return True
    else:
        print(r.json())
        return False


def checkForRunCommand(config):
    url = config.get(CONFIG_OUTLET_URL)
    if not url:
        return None

    connectToWifi(config)

    try:
        r = urequests.get(url)
    except Exception as e:
        print("Unable to access: {}".format(url))
        print(str(e))
        return None

    if r.text:
        results = r.json()
        if MEASUREMENT in results:
            if results[MEASUREMENT] == True:
                return True
            else:
                return False    
    return None


def connectToWifi(config):
    while not WIFI.isconnected():
        ssid = config.get(CONFIG_WIFI_SSID)
        passwd = config.get(CONFIG_WIFI_PASSWD)
        print("Connecting to {}".format(ssid))
        WIFI.connect(ssid, passwd)
        for x in range(CONNECTION_DELAY_SEC):
            if WIFI.isconnected():
                print("Connected:")
                print(WIFI.ifconfig())
                return
            else:
                print(".")
                time.sleep(1)


def remoteOutlet():
    # Application Start Point
    outlet_status = machine.Pin(OUTLET_SENSE_PIN, machine.Pin.IN, machine.Pin.PULL_UP)
    outlet_control = machine.Pin(OUTLET_CONTROL_PIN, machine.Pin.OUT)
    # relay control is inverted
    outlet_control.on()

    config = loadConfig()
    connectToWifi(config)
    while True:
        run = checkForRunCommand(config)
        if run is not None:
            if run == True:
                print("Outlet Control: ON")
                # relay control is inverted
                outlet_control.off()
            else:
                print("Outlet Control: OFF")
                # relay control is inverted
                outlet_control.on()

        # The optocoupler's output is inverted
        value = 1 - outlet_status.value()
        print("Outlet status: {}".format(value))
        sendDatapoint(config, value)

        for x in range(DATA_DELAY_SEC):
            time.sleep(1)


try:
    remoteOutlet()
except Exception as e:
    # Reset on error
    machine.reset()
