from mbed_host_tests import BaseHostTest, event_callback
from ubirch.ble_platform import get_scanner, get_manager
import time


class BLEUartServiceTests(BaseHostTest):
    """
    BLE UART Service Tests

    Check discovery of the UART characteristic and
    writing to it, as well as reading from it.
    """

    def __init__(self):
        self.log("** " + str(self))
        self.sm = get_scanner()
        BaseHostTest.__init__(self)

        self.adapter = ""
        self.addressType = "random"
        self.securityLevel = "low"
        self.createRequester = False


    def discoverDevice(self, name):
        self.log("** [U] discoverDevice(" + name + ")")
        return self.sm.getDeviceAddress(name, 5)


    @event_callback("discover")
    def __discover(self, key, name, timestamp):
        self.log("** [U] " + key + "(" + name + ")")

        # discover characteristics in peripheral
        self.device  = self.discoverDevice(name)
        self.cm = get_manager(self.device, self.adapter, self.addressType, self.securityLevel, self.createRequester)
        self.cm.connectDevice()
        peripheral = self.cm.discoverCharacteristics()
        for c in peripheral:
            self.send_kv('hello', c.upper())
            time.sleep(0.5)

        # disconnect from peripheral
        self.cm.disconnectDevice()

    @event_callback("writedata")
    def __writedata(self, key, name, timestamp):
        self.log("** [U] " + key + "(" + name + ")")

        # discover, setup and connect to remote device
        self.device = self.discoverDevice(name)
        self.cm = get_manager(self.device, self.adapter, self.addressType, self.securityLevel, self.createRequester)
        self.cm.connectDevice()
        peripheral = self.cm.discoverCharacteristics()

        # send the uuid to be expected via BLE, then send it
        self.send_kv("expect", peripheral[0])
        # TODO get the handle from discoverService function
        self.cm.write(0xe, peripheral[0])

        # disconnect from peripheral
        self.cm.disconnectDevice()

    @event_callback("readdata")
    def __readdata(self, key, name, timestamp):
        self.log("** [U] " + key + "(" + name + ")")

        # discover, setup and connect to remote device
        self.device = self.discoverDevice(name)
        self.cm = get_manager(self.device, self.adapter, self.addressType, self.securityLevel, self.createRequester)
        self.cm.connectDevice()
        peripheral = self.cm.discoverCharacteristics()

        # send the uuid to be expected via BLE, then wait for it
        self.send_kv("expect", peripheral[0])
        time.sleep(0.5)
        # TODO get the handle from discoverService function
        c = self.cm.read(0x10)
        self.send_kv("received", c[0])

        # try again
        self.send_kv("expect", peripheral[0])
        time.sleep(0.5)
        # TODO get the handle from discoverService function
        c2 = self.cm.read(0x10)

        # TODO Fix the buffer issue with blesuite
        c2 = c2[0][:len(c[0])]
        self.send_kv("received", c2)

        # disconnect from peripheral
        self.cm.disconnectDevice()


# class GenericProfileHandler(DefaultProfileHandler):
#     UUID = "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
#     _AUTOLOAD = True
#     names = {
#         "6E400001-B5A3-F393-E0A9-E50E24DCCA9E": "UART Profile",
#         "6E400002-B5A3-F393-E0A9-E50E24DCCA9E": "UART TX",
#         "6E400003-B5A3-F393-E0A9-E50E24DCCA9E": "UART RX"
#     }
#
#     def initialize(self):
#         print "init"
#         pass
#
#     def on_write(self, characteristic, data):
#         print "** write(" + str(characteristic.UUID) + "): '"+("".join(data))+"'"
#
#
#     def on_read(self, characteristic, data):
#         print "** read(" + str(characteristic.UUID) + "): '"+("".join(data))+"'"
#
#     def on_notify(self, characteristic, data):
#         print "** notify(" + str(characteristic.UUID) + "): '"+("".join(data))+"'"
#
#
# class Peripheral(PeripheralHandler):
#     def initialize(self):
#         self.addProfileHandler(GenericProfileHandler)
#         pass
#
#     def on_connect(self):
#         print "** connect(", self.peripheral, ")"
#         pass
#
#     def on_disconnect(self):
#         print "** disconnect(", self.peripheral, ")"
#         pass
#
#     def on_rssi(self, value):
#         print "** updateRSSI(", self.peripheral, ",", value, ")"
#         pass
