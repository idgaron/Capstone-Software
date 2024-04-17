import serial
import matplotlib.pyplot as plt
import numpy as np

# Parameters
port = '/dev/cu.wchusbserial57710030221'
baudrate = 115200
window_size = 100  # Number of points to display in the plot
sampling_frequency = 20000  # Sampling frequency in Hz
fft_frequency_range = (0, sampling_frequency / 2)  # Nyquist frequency
update_interval = 10  # Update plot every 10 data points

# Initialize serial connection
ser = serial.Serial(port, baudrate)

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

ax2.set_xlim(0, sampling_frequency / 2)  # Adjusted for sampling frequency
ax2.set_ylim(0, 100)  # Adjust as needed
ax2.set_xlabel('Frequency (Hz)')
ax2.set_ylabel('Magnitude')
ax2.set_title('FFT')

# Main loop for real-time plotting
try:
    xdata = np.arange(window_size)
    ydata = np.zeros(window_size)
    count = 0
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
            ydata = np.append(ydata, y)[-window_size:]
            count += 1
            
            if count % update_interval == 0:
                # Adjust x-axis limits to move the window
                ax1.set_xlim(xdata[0], xdata[-1])
                
                # Update the plot with the new data
                line1.set_data(xdata, ydata)

                fft_data = np.fft.fft(ydata)
                freq = np.fft.fftfreq(window_size, d=1/sampling_frequency)  # Adjusted for sampling frequency
                
                # Update the FFT subplot
                line2.set_data(freq, np.abs(fft_data))  # Plot full frequency range
                
                # Adjust x-axis limits of the FFT subplot
                ax2.set_xlim(fft_frequency_range)

                plt.draw()
                plt.pause(0.0001)  # Pause to update plot
        except ValueError:
            pass

except KeyboardInterrupt:
    plt.close()

# Close serial connection
ser.close()
