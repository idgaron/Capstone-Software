import serial
import matplotlib.pyplot as plt
import matplotlib.animation as animation
from matplotlib import style

s = serial.Serial('/dev/cu.wchusbserial57710030221', 115200)

style.use('fivethirtyeight')

fig = plt.figure()
ax1 = fig.add_subplot(1,1,1)

def update_plot(x,y):
    ax1.clear()
    ax1.plot(x, y)
    plt.draw()
    plt.pause(0.00001)



rp2040_data_list = []
x = []
y = []
i = 0

while True:
    rp2040_data = s.read().decode()
    rp2040_data_list.append(str(rp2040_data))
    if rp2040_data == "\n": 
        i += 1
        rp2040_data_list = ''.join(rp2040_data_list)
        rp2040_data_split = rp2040_data_list.split(' ')
        #print(rp2040_data_split)
        x.append(float(rp2040_data_split[2]))
        #rp2040_data_list = []
        y.append(float(rp2040_data_split[5]))
        #if len(x) > 10:
        #    x.pop(0)

        #if len(y) > 10:
        #    y.pop(0)
        
        update_plot(x,y)
        #ax1.clear()
        #ax1.plot(x, y)
        #if i > 1000:
        #    update_plot(x,y)
        #print(y)
        #print(rp2040_data_list)
        rp2040_data_list = []
        

