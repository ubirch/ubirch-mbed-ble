from mbed_host_tests import BaseHostTest, event_callback
from threading import Thread
from bleep import BLEDevice

class BLEManagerTests(BaseHostTest):
    def __init__(self):
        BaseHostTest.__init__(self)

    def discoverDevice(self, name):
        for i in xrange(5):
            for device in BLEDevice.discoverDevices(timeout=10):
                self.log("discovered: " + str(device))
                if device.name == name:
                    return device
        raise RuntimeError("no devices found")

    def __discover(self, name):
        try:
            device = self.discoverDevice(name)
            self.log("found " + str(device))
            self.send_kv("discovered", device.name)
        except Exception as e:
            self.log("ERROR: did not find device: " + str(e))
            self.send_kv("discovered", "--NOTFOUND--")

    @event_callback("discover")
    def _callback_discover(self, key, value, timestamp):
        self.log("discovering BLE device '" + value + "'")
        self.callbackThread = Thread(target=self.__discover, args=(value,))
        self.callbackThread.daemon = True
        self.callbackThread.start()

    def __connect(self, name):
        try:
            device = self.discoverDevice(name)
            self.log("connecting to " + str(device))
            device.connect()
            device.disconnect()
        except Exception as e:
            self.log("ERROR: did not find device: " + str(e))
            self.send_kv("connected", "--NOTFOUND--")


    @event_callback("connect")
    def _callback_connect(self, key, value, timestamp):
        self.log("connect to BLE device '" + value + "'")
        self.connectThread = Thread(target=self.__connect, args=(value,))
        self.connectThread.daemon = True
        self.connectThread.start()

    @event_callback("connected")
    def _callback_connection(self, key, value, timestamp):
        self.log("connected to BLE device '" + value + "'")
        self.send_kv("connected", value)
