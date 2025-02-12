from machine import Pin, UART
import time
import struct
import adafruit_lsm9ds1

uart = UART(1, baudrate=115200, tx=Pin(4), rx=Pin(5))

from machine import I2C
i2c = I2C(0, scl=Pin(22), sda=Pin(21))
sensor = adafruit_lsm9ds1.LSSM9DS1_I2C(i2c)

def send_sensor_data():
    accel_x, accel_y, accel_z = sensor.acceleration
    gyro_x, gryo_y, gryo_z = sensor.gyro
    mag_x, mag_y, mag_z = sensor.magnetic
    
    data = struct.pack('9f', accel_x, accel_y, accel_z, gyro_x, gyro_y, gyro_z, mag_x, mag_y, mag_z)
    
    uart.write(data)
    
while True:
    send_sensor_data()
    time.sleep(1)