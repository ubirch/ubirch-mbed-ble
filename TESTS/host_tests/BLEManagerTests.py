from mbed_host_tests import BaseHostTest, event_callback
from pyble.handlers import DefaultProfileHandler, PeripheralHandler

from ubirch.ble_platform import get_scanner, get_manager


class BLEManagerTests(BaseHostTest):
    """
    BLE Manager Tests

    Simple Tests to check whether we can see the BLE device
    and are able to connect to it.
    """

    def __init__(self):
        self.log("** " + str(self))
        self.sm = get_scanner()
        BaseHostTest.__init__(self)

        # address = test_basic()
        self.adapter = ""
        self.addressType = "random"
        self.securityLevel = "low"
        self.createRequester = False

    def discoverDevice(self, name):
            self.log("** [B] discoverDevice(" + name + ")")
            return self.sm.getDeviceAddress(name, 5)

    @event_callback("discover")
    def __discover(self, key, value, timestamp):
        self.log("** [B] " + key + "(" + value + ")")

        # TODO return device name for linux devices
        d = self.discoverDevice(value)
        if d:
            self.send_kv("discovered", value.encode("latin-1"))
        else:
            self.send_kv("discovered", "NONE")


    @event_callback("connect")
    def __connect(self, key, value, timestamp):
        self.log("** [B] " + key + "(" + value + ")")
        self.device = self.discoverDevice(value)
        self.cm = get_manager(self.device, self.adapter, self.addressType, self.securityLevel, self.createRequester)
        self.cm.connectDevice()

    @event_callback("connected")
    def __connected(self, key, value, timestamp):
        self.log("** [B] " + key + "(" + value + ")")
        self.send_kv("connected", value)
        self.cm.disconnectDevice()

    @event_callback("disconnected")
    def __disconnected(self, key, value, timestamp):
        self.log("** [B] " + key + "(" + value + ")")
        self.send_kv("disconnected", value)

    # TODO implement secure BLE
    @event_callback("connect_secure")
    def __connect_secure(self, key, value, timestamp):
        self.log("** [B] " + key + "(" + value + ")")
        self.device = self.discoverDevice(value)
        self.device.delegate = SecurePeripheral
        self.peripheral = self.cm.connectPeripheral(self.device)
        self.send_kv("expect", 'X')
        # read data ...
        c = self.peripheral["TESTService"]["SecureChar"]
        c.notify = True
        self.cm.loop(5)
        self.send_kv("received", str(c.value))
        self.send_kv("finished", "OK")
        self.cm.disconnectPeripheral(self.device)

    @event_callback("passkey")
    def __secured(self, key, value, timestamp):
        self.log("** [B] " + key + "(" + value + ")")

    @event_callback("secured")
    def __secured(self, key, value, timestamp):
        self.log("** [B] " + key + "(" + value + ")")
        # self.send_kv("secured", str(value))

class GenericProfileHandler(DefaultProfileHandler):
    UUID = "C30F7571-116B-4BF2-8EB3-B97CD531A113"
    _AUTOLOAD = True
    names = {
        "C30F7571-116B-4BF2-8EB3-B97CD531A113": "TESTService",
        "C30F7572-116B-4BF2-8EB3-B97CD531A113": "SecureChar",
    }

    def initialize(self):
        print "** init"
        pass

    def on_write(self, characteristic, data):
        print "** write(" + str(characteristic.UUID) + "): '"+("".join(data))+"'"


    def on_read(self, characteristic, data):
        if not isinstance(data, (frozenset, list, set, tuple,)):
            print "** read(" + str(characteristic.UUID) + "): '"+str(data)+"'"
        else:
            print "** read(" + str(characteristic.UUID) + "): '"+("".join(data))+"'"

    def on_notify(self, characteristic, data):
        print "** notify(" + str(characteristic.UUID) + "): '"+("".join(data))+"'"


class SecurePeripheral(PeripheralHandler):
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
