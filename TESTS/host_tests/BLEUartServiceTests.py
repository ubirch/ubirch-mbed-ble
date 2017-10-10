from mbed_host_tests import BaseHostTest, event_callback
from pyble import CentralManager
from pyble.handlers import PeripheralHandler, ProfileHandler

class BLEUartServiceTests(BaseHostTest):
    """
    BLE UART Service Tests

    Check discovery of the UART characteristic and
    writing to it, as well as reading from it.
    """

    def __init__(self):
        self.log("** " + str(self))
        self.cm = CentralManager()
        BaseHostTest.__init__(self)

    def teardown(self):
        self.log("** teardown()")
        pass

    def discoverDevice(self, name):
        self.log("** discoverDevice(" + name + ")")
        if not self.cm.ready:
            return
        try:
            target = self.cm.startScan(timeout=10)
            if target and target.name == name:
                return target
        finally:
            self.cm.stopScan()
        raise Exception("NO DEVICE FOUND")

    @event_callback("discover")
    def __discover(self, key, name, timestamp):
        self.log("** " + key + "(" + name + ")")

        # discover characteristics in peripheral
        self.device = self.discoverDevice(name)
        peripheral = self.cm.connectPeripheral(self.device)
        for service in peripheral:
            for c in service:
                self.send_kv(c.name, c.UUID)
        self.cm.disconnectPeripheral(self.device)

    @event_callback("writedata")
    def __writedata(self, key, name, timestamp):
        self.log("** " + key + "(" + name + ")")

        # discover, setup and connect to remote device
        self.device = self.discoverDevice(name)
        self.device.delegate = Peripheral
        peripheral = self.cm.connectPeripheral(self.device)

        # send the uuid to be expected via BLE, then send it
        self.send_kv("expect", self.device.services[0].UUID)
        c = peripheral["UART Profile"]["UART TX"]
        c.value = bytearray(self.device.services[0].UUID)
        self.cm.disconnectPeripheral(self.device)


class GenericProfileHandler(ProfileHandler):
    UUID = "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
    _AUTOLOAD = True
    names = {
        "6E400001-B5A3-F393-E0A9-E50E24DCCA9E": "UART Profile",
        "6E400002-B5A3-F393-E0A9-E50E24DCCA9E": "UART TX",
        "6E400003-B5A3-F393-E0A9-E50E24DCCA9E": "UART RX"
    }

    def initialize(self):
        print "init"
        pass

    def on_write(self, characteristic, data):
        print "** write(" + str(characteristic.UUID) + ")"

    def on_read(self, characteristic, data):
        print "** read(" + str(characteristic.UUID) + ")"
        ans = []
        for b in data:
            ans.append("%02X" % ord(b))
        ret = "0x" + "".join(ans)
        return ret

    def on_notify(self, characteristic, data):
        print "** notify(" + str(characteristic.UUID) + ")"


class Peripheral(PeripheralHandler):
    def initialize(self):
        self.addProfileHandler(GenericProfileHandler)
        pass

    def on_connect(self):
        print "** connect(", self.peripheral, ")"
        pass

    def on_disconnect(self):
        print "** disconnect(", self.peripheral, ")"
        pass

    def on_rssi(self, value):
        print "** updateRSSI(", self.peripheral, ",", value, ")"
        pass
