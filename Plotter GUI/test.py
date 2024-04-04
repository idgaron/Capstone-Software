import serial
import matplotlib.pyplot as plt
import numpy as np

# Parameters
port = '/dev/cu.usbmodem102'  # Change this to your serial port
baudrate = 115200

# Initialize serial connection
ser = serial.Serial(port, baudrate)


while True:
        # Read data from serial port
    raw_data = ser.readline().decode().strip()
    rp2040_data_split = raw_data.split(' ')
    print(rp2040_data_split[1])
        