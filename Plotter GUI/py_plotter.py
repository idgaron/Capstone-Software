import serial
import matplotlib.pyplot as plt
import numpy as np

# Parameters
port = '/dev/cu.wchusbserial57710030221'  # Change this to your serial port
baudrate = 115200
window_size = 1000  # Number of points to display in the plot
fft_frequency_range = (0, 10000)

# Initialize serial connection
ser = serial.Serial(port, baudrate)

# Create figure and axis objects
# Create figure and axis objects
plt.ion()  # Turn on interactive mode
fig, (ax1, ax2) = plt.subplots(2, 1)
line1, = ax1.plot([], [], 'r-')
line2, = ax2.plot([], [], 'b-')

# Initialize plot
ax1.set_xlim(0, window_size)
ax1.set_ylim(-2, 2)  # Assuming data range from 0 to 1023
ax1.set_xlabel('Time')
ax1.set_ylabel('Data')
ax1.set_title('Real-time Serial Plot')
ax1.grid(True)

ax2.set_xlim(fft_frequency_range)  # Frequency range up to Nyquist frequency
ax2.set_ylim(0, 1000)  # Adjust as needed
ax2.set_xlabel('Frequency (Hz)')
ax2.set_ylabel('Magnitude')
ax2.set_title('FFT')

# Main loop for real-time plotting
try:
    xdata = []
    ydata = []
    while True:
        # Read data from serial port
        raw_data = ser.readline().decode().strip()
        rp2040_data_split = raw_data.split(' ')
        # Ensure there are enough elements in the split data
        if len(rp2040_data_split) < 6:
            continue  # Skip this iteration if data is incomplete

        try:
            x = float(rp2040_data_split[1])
            y = float(rp2040_data_split[5])
            # Append new data
            xdata.append(x)
            ydata.append(y)
            

            # Truncate data if it exceeds the window size
            if len(xdata) > window_size:
                ax1.set_xlim(xdata[-window_size], xdata[-1])
            
            # Update the plot with the new data
            line1.set_data(xdata[-window_size:], ydata[-window_size:])

            fft_data = np.fft.fft(ydata[-window_size:])
            freq = np.fft.fftfreq(window_size, d=1/baudrate)
            
            # Update the FFT subplot
            line2.set_data(freq, np.abs(fft_data))  # Plot full frequency range
            
            # Adjust x-axis limits of the FFT subplot
            ax2.set_xlim(fft_frequency_range)

            plt.draw()
            plt.pause(0.00000001)  # Pause to update plot
        except ValueError:
            pass

except KeyboardInterrupt:
    plt.close()

# Close serial connection
ser.close()
