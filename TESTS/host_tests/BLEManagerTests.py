from mbed_host_tests import BaseHostTest, event_callback
from pyble import CentralManager

class BLEManagerTests(BaseHostTest):
    """
    BLE Manager Tests

    Simple Tests to check whether we can see the BLE device
    and are able to connect to it.
    """
    def __init__(self):
        self.cm = CentralManager()
        BaseHostTest.__init__(self)

    def discoverDevice(self, name):
        if not self.cm.ready:
            return
        for i in xrange(5):
            target = self.cm.startScan()
            if target and target.name == name:
                return target
        raise Exception("NO DEVICE FOUND")

    @event_callback("discover")
    def __discover(self, key, value, timestamp):
        self.log("** " + key + "(" + value + ")")
        device = self.discoverDevice(value)
        self.send_kv("discovered", device.name.encode("latin-1"))

    @event_callback("connect")
    def __connect(self, key, value, timestamp):
        self.log("** " + key + "(" + value + ")")
        self.device = self.discoverDevice(value)
        self.cm.connectPeripheral(self.device)

    @event_callback("connected")
    def __connected(self, key, value, timestamp):
        self.log("** " + key + "(" + value + ")")
        self.send_kv("connected", value)
        self.cm.disconnectPeripheral(self.device)

    @event_callback("disconnected")
    def __disconnected(self, key, value, timestamp):
        self.log("** " + key + "(" + value + ")")
        self.send_kv("disconnected", value)

