from mbed_host_tests import BaseHostTest, event_callback
from time import sleep
from bleep import BLEDevice

class BLEManagerTests(BaseHostTest):
    def __init__(self):
        BaseHostTest.__init__(self)


    @event_callback("discover")
    def _callback_host_ip(self, key, value, timestamp):
        self.log("discovering BLE device '" + value + "'")

        # try five times to find the device
        for i in xrange(5):
            for device in BLEDevice.discoverDevices():
                print(device)
                if device.name == value:
                    self.send_kv("discovered", value)
                    return
            sleep(2)

        self.send_kv("discovered", "--NOTFOUND--")
