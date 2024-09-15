from machine import I2C
from machine import Pin
from machine import PWM
import time
from si4735.si4735_constants import *

import logging
import micropython

logging.basicConfig(level=logging.DEBUG)
logger = logging.getLogger(__name__)

micropython.alloc_emergency_exception_buf(100)

class MyHandler(logging.Handler):
    def emit(self, record):
        # for key, value in record.__dict__.items():
        #     print(f"{key}={value}", end=" ")
        print("levelname=%(levelname)s name=%(name)s message=%(msg)s" % record.__dict__)
        pass

logger.addHandler(MyHandler())




# Properties
REFCLK_FREQ = 0x201

REFCLK_PRESCALE = 0x0202
PRESCALER = 4
USE_DCLK = 0x00

RESET_PIN = 4
SCLK_PIN = 3
SDIO_PIN = 2
XOSCEN_USE_CRYSTAL = 0x10
XOSCEN_USE_EXTERNAL_OSCILLATOR = 0x00
USE_EXTERNAL_OSCILLATOR = True
OSCILLATOR_PIN = 5
OSCILLATOR_FEQUENCY = 131000
REFCLOCK_SCALE = 4
REFCLOCK_FREQ = int(OSCILLATOR_FEQUENCY/REFCLOCK_SCALE)
OPMODE = 0x03
FM = 0
AM = 1


class SI4735:
    def __init__(self, i2c_bus=1, address=0x63):
        logger.debug("Initializing SI4735 class")
        self.reset_pin = Pin(RESET_PIN, Pin.OUT)
        if USE_EXTERNAL_OSCILLATOR:
            self.oscillator_pin = PWM(
                Pin(OSCILLATOR_PIN, Pin.OUT), freq=OSCILLATOR_FEQUENCY, duty_u16=32768)

        self.reset_pin.value(1)
        self.bus = I2C(id= 1, freq=100000, scl=SCLK_PIN, sda=SDIO_PIN)
        self.address = None  # ???
        self.mode = None

    def get_device_address(self):
        """finds the address of the SI4735 device on the I2C bus.
        possible values are 0x11 and 0x63
        Args:
            None
        Returns:
            int: address of the found device or 0 if no device is found
        """
        logger.debug("Finding device address")
        active_addresses = self.bus.scan()
        if 0x11 in active_addresses:
            self.address = 0x11
        elif 0x63 in active_addresses:
            self.address = 0x63
        else:
            self.address = 0
        return self.address

    def reset(self):
        logger.debug("Resetting SI4735")
        self.reset_pin.value(0)
        time.sleep(0.1)
        self.reset_pin.value(1)

    def wait_for_ready(self):
        logger.debug("Waiting for SI4735 to be ready")
        while True:
            status = self.get_status()
            if status[0] & 0x80:
                if status[0] & 0x40:
                    logger.warning("Device is in error state")
                break
            time.sleep(0.01)
        return status[0]

    def set_mode(self, mode):
        logger.debug(f"Setting mode to {hex(mode)}")
        self.wait_for_ready()
        if mode == FM and self.mode != FM:
            self.mode = FM
            self.power_up('FM')
        elif mode == AM and self.mode != AM:
            self.mode = AM
            self.power_up('AM')

    def set_AM_mode(self, band_bottom, band_top):
        logger.debug("Setting AM mode")
        if self.mode == AM:
            return
        self.power_down()
        self.power_up(AM)
        self.set_property(AM_SEEK_BAND_BOTTOM, band_bottom)
        self.set_property(AM_SEEK_BAND_TOP, band_top)
        self.set_volume(30)
        
    def set_FM_mode(self, band_bottom, band_top):
        logger.debug("Setting FM mode")
        if self.mode == FM:
            return
        self.power_down()
        self.power_up(FM)
        self.set_property(FM_SEEK_BAND_BOTTOM, band_bottom)
        self.set_property(FM_SEEK_BAND_TOP, band_top)
        self.set_volume(30)

    def power_up(self, mode=FM):
        logger.debug(f"Powering up in {mode} mode")
        b = bytearray([POWER_UP, 0x50, 0x05]) # FM mode
        if mode == AM:
            b[1] = 0x51
        self.bus.writeto(self.address, b)
        time.sleep(0.1 if USE_EXTERNAL_OSCILLATOR else 0.5)
        self.set_property(REFCLK_FREQ, REFCLOCK_FREQ)
        self.set_property(REFCLK_PRESCALE, REFCLOCK_SCALE)
        time.sleep(0.1)

    def power_down(self):
        logger.debug("Powering down")
        b = bytearray([0x11])
        self.bus.writeto(self.address, b)
        time.sleep(0.1)

    def set_property(self, property, value):
        logger.debug(f"Setting property {hex(property)} to {hex(value)}")
        b = bytearray([SET_PROPERTY, 0x00, property >> 8,
                 property & 0xFF, value >> 8, value & 0xFF])
        self.bus.writeto(self.address, b)
        self.wait_for_ready()
        
    def get_property(self, property):
        logger.debug(f"Getting property {hex(property)}")
        b = bytearray([SET_PROPERTY, 0x00, property >> 8, property & 0xFF])
        self.bus.writeto(self.address, b)
        self.wait_for_ready()
        result = self.bus.readfrom(self.address, 3)
        logger.debug(f"Result: {result}")
        return result
    
    def get_version(self):
        logger.debug("Getting version")
        b = bytearray([GET_REV])
        self.bus.writeto(self.address, b)
        self.wait_for_ready()
        result = self.bus.readfrom(self.address, 8)
        logger.debug(f"Part No: {result[0]:d}, Firmware: {result[1]:d}.{result[2]:d}, Patch: {result[3:4]:d}, Component: {result[5]:d}.{result[6]:d}")
        return result
    
    def get_int_status(self):
        logger.debug("Getting interrupt status")
        b = bytearray([GET_INT_STATUS])
        self.bus.writeto(self.address, b)
        result = self.wait_for_ready()
        logger.debug(f"Result: {hex(result)}")
        return result
    
    def set_band_limits(self, band_bottom, band_top):
        logger.debug(f"Setting band limits to {band_bottom} and {band_top}")
        b = bytearray([SET_BAND_LIMITS, 0x00, band_bottom >> 8, band_bottom & 0xFF, band_top >> 8, band_top & 0xFF])
        self.bus.writeto(self.address, b)
        self.wait_for_ready()
        

    def fm_seek_start(self, ):
        logger.debug(f"Setting FM seek start to {frequency} MHz")
        b = bytearray([0x21, 0x00, frequency >> 8, frequency & 0xFF])
        self.bus.writeto(self.address, b)
        self.wait_for_ready()
        time.sleep(0.1)

    def tune_fm(self, frequency):
        logger.debug(f"FM tuning to {frequency} MHz")
        logger.debug(f"high byte: {hex(frequency>>8)}, low byte: {hex(frequency & 0xFF)}")
        b = bytearray([FM_TUNE_FREQ, 0x00, frequency >> 8, frequency & 0xFF])
        self.bus.writeto(self.address, b)
        while self.get_int_status() & 1 == 0:
            pass
        self.wait_for_ready()

    def tune_am(self, frequency):
        logger.debug(f"AM tuning to {frequency} kHz")
        b = bytearray([AM_TUNE_FREQ, 0x00, frequency >> 8, frequency & 0xFF],0x00)
        self.bus.writeto(self.address, b)
        while self.get_int_status() & 1 == 0:
            pass
        self.wait_for_ready()

    def set_volume(self, volume):
        logger.debug(f"Setting volume to {volume}")
        self.set_property(0x4000, volume)
        self.wait_for_ready()

    def get_status(self):
        logger.debug("Getting status")
        b = bytearray([GET_INT_STATUS])
        self.bus.writeto(self.address, b)
        result = self.bus.readfrom(self.address, 1)
        logger.debug(f"Status: {hex(result[0])}")
        return result

    def get_tune_status(self):
        logger.debug("Getting tune status")
        b = bytearray([FM_TUNE_STATUS if self.mode ==
                      FM else AM_TUNE_STATUS, 1]) # 1 = INTACK
        self.bus.writeto(self.address, b)
        self.wait_for_ready()
        result = self.bus.readfrom(self.address, 7)
        logger.debug(f"Result: {result}")
        return result

    def wait_for_ready(self):
        logger.debug("Waiting for SI4735 to be ready")
        while True:
            status = self.get_status()
            if status[0] & 0x80:
                if status[0] & 0x40:
                    logger.warning("Device is in error state")
                break
            time.sleep(0.01)
        return status[0]
    
    def get_rev(self):
        logger.debug("Getting revision")
        b = bytearray([0x10])
        self.bus.writeto(self.address, b)
        result = self.bus.readfrom(self.address, 14)
        logger.debug(f"Result: {result}")
        return result

    def get_rsq_status(self):
        logger.debug("Getting RSQ status")
        b = bytearray([0x23, 0x01])
        self.bus.writeto(self.address, b)
        result = self.bus.readfrom(self.address, 7)
        logger.debug(f"Result: {result}")
        return result

# Example usage
if __name__ == "__main__":
    print("Running the wrong file")