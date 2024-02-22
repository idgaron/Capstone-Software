import serial
import pyqtgraph

s = serial.Serial('/dev/cu.wchusbserial57710030221', 115200)

rp2040_data = s.read().decode()




