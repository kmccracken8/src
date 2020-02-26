import usb.core
import time

class modeselect:

    def __init__(self):
        self.SPRING = 0
        self.DAMPER = 1
        self.TEXTURE = 2
        self.WALL = 3
        self.dev = usb.core.find(idVendor = 0x6666, idProduct = 0x0003)
        if self.dev is None:
            raise ValueError('no USB device found matching idVendor = 0x6666 and idProduct = 0x0003')
        self.dev.set_configuration()

    def close(self):
        self.dev = None

    def spring(self):
        try:
            self.dev.ctrl_transfer(0x40, self.SPRING)
        except usb.core.USBError:
            print("Could not send SPRING vendor request.")

    def damper(self):
        try:
            self.dev.ctrl_transfer(0x40, self.DAMPER)
        except usb.core.USBError:
            print("Could not send SPRING vendor request.")

    def texture(self):
        try:
            self.dev.ctrl_transfer(0x40, self.TEXTURE)
        except usb.core.USBError:
            print("Could not send SPRING vendor request.")

    def wall(self):
        try:
            self.dev.ctrl_transfer(0x40, self.WALL)
        except usb.core.USBError:
            print("Could not send SPRING vendor request.")
