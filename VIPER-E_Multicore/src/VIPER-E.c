#include "VIPER-E.h"
#include <string.h>

/*
* Initializes BNO-055. When the BNO-055 is powered,
* it is switched from configuration mode to IMU mode, and
* the units are set to m/s^2 for the accelerometer.
*/
static void bnoReset() {
    // switches BNO into IMU mode
    uint8_t config_buf[] = {(uint8_t)CONFIGURATION_REGISTER, (uint8_t)IMU_MODE};
    i2c_write_blocking(i2c_default, BNO055_ADDRESS, config_buf, 2, false);
    sleep_ms(30); // takes 30ms for changes to take place

    // sets the units to m/s^2 for the accelerometer
    uint8_t unit_buf[] = {(uint8_t)UNIT_REGISTER, (uint8_t)UNITS};
    i2c_write_blocking(i2c_default, BNO055_ADDRESS, unit_buf, 2, false);
} // bnoReset

/*
* Reads the Z axis of the BNO-055. Reads the 2 bytes acceleration
* value from the Z register and returns a single int.
*
* @return accel_z - current acceleration in the z axis
*/
int16_t bnoReadZ() {
    uint8_t buffer[2];
    uint8_t address = ACCEL_Z_AXIS;

    // writes to the IMU to tell it what address it wants to read from
    i2c_write_blocking(i2c_default, BNO055_ADDRESS, &address, 1, true);
    i2c_read_blocking(i2c_default, BNO055_ADDRESS, buffer, 2, false); // reads 2 bytes acceleration value

    return (buffer[1] << 8 | buffer[0]); // returns a single 16 bit integer
} // bnoReadZ

/*
* Initializes Real Time Clock (RTC). The RTC is used to time the
* execution of the program for a reliable start/stop time.
*
* @param date - a datetime_t struct intializing starting time
*/
void initializeRTC(datetime_t date) {
    // Start the RTC
    rtc_init();
    rtc_set_datetime(&date);
} // initializeRTC

/*
* Initializes the Analog to Digital Converter (ADC).
*   
* @param GPIO - the GPIO number of the ADC
* @param channel - the channel number of the ADC 
*/
void initalizeADC(int GPIO, int channel) {
    adc_init();
    adc_gpio_init(GPIO);
} // initializeADC

/*
* Initializes I2C communications for the BNO055 Sensor. Uses the default 
* SDA and SCL pins on the pico (GPIO 4 (pin 6) and 5 (pin 7)) with a baud
* rate of 400000.
*/
void initializeI2C() {
    i2c_init(i2c_default, 400 * 1000);
    // changed defaults in pico.h --> (SDA = 4 --> 2, SCL = 5 --> 3, CHAN 0 --> 1)
    gpio_set_function(PICO_DEFAULT_I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(PICO_DEFAULT_I2C_SDA_PIN);
    gpio_pull_up(PICO_DEFAULT_I2C_SCL_PIN);
    
    // Make the I2C pins available to picotool
    bi_decl(bi_2pins_with_func(PICO_DEFAULT_I2C_SDA_PIN, PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C));

    // reset the BNO055
    bnoReset();
} // initializeI2C

void initializeGPIO(int GPIO, bool direction) {
    gpio_init(GPIO);
    gpio_set_dir(GPIO, direction);
    gpio_put(GPIO, 0);
}

#define BUFFER_SIZE 512
volatile bool core0_finished = false;

// Define a structure for the circular buffer
struct CircularBuffer {
    uint16_t mfc_control[BUFFER_SIZE];
    uint16_t mfc_experimental[BUFFER_SIZE];
    int64_t time_dif[BUFFER_SIZE];
    volatile uint8_t write_index;
    volatile uint8_t read_index;
};

// Declare the circular buffer
struct CircularBuffer data_buffer;

// Initialize the circular buffer
void initializeCircularBuffer() {
    data_buffer.write_index = 0;
    data_buffer.read_index = 0;
}

void clearCircularBuffer() {
    data_buffer.write_index = 0;
    data_buffer.read_index = 0;
}

// Check if the circular buffer is full
bool isBufferFull() {
    return ((data_buffer.write_index + 1) % BUFFER_SIZE == data_buffer.read_index);
}

// Check if the circular buffer is empty
bool isBufferEmpty() {
    return (data_buffer.write_index == data_buffer.read_index);
}

// Add data to the circular buffer
bool addToBuffer(uint16_t mfc_control, uint16_t mfc_experimental, int64_t time_dif) {
    if (!isBufferFull()) {
        data_buffer.mfc_control[data_buffer.write_index] = mfc_control;
        data_buffer.mfc_experimental[data_buffer.write_index] = mfc_experimental;
        data_buffer.time_dif[data_buffer.write_index] = time_dif;
        data_buffer.write_index = (data_buffer.write_index + 1) % BUFFER_SIZE;
        return true; // Successfully added to the buffer
    }
    return false; // Buffer is full
}

// Remove data from the circular buffer
bool removeFromBuffer(uint16_t *mfc_control, uint16_t *mfc_experimental, int64_t *time_dif) {
    if (!isBufferEmpty()) {
        *mfc_control = data_buffer.mfc_control[data_buffer.read_index];
        *mfc_experimental = data_buffer.mfc_experimental[data_buffer.read_index];
        *time_dif = data_buffer.time_dif[data_buffer.read_index];
        data_buffer.read_index = (data_buffer.read_index + 1) % BUFFER_SIZE;
        return true; // Successfully removed from the buffer
    }
    return false; // Buffer is empty
}

// Calculate the number of bytes in the circular buffer
int bytes_in_buffer() {
    int num_elements = (data_buffer.write_index >= data_buffer.read_index) ?
                       (data_buffer.write_index - data_buffer.read_index) :
                       (BUFFER_SIZE - data_buffer.read_index + data_buffer.write_index);

    int num_bytes = num_elements * (sizeof(uint16_t) * 2 + sizeof(int64_t));
    return num_bytes;
}

// Setup up method for Core 1
void core_1(){
    // variables to access and write to file
    FRESULT fileResult;
    FATFS fileSystem0, fileSystem1;
    FIL file0, file1;
    int ret;
    char buf[100];
    char filename0[] = "0:/TEST0.csv";
    char filename1[] = "1:/TEST1.csv";

    // Start at 0 for H:M:S
    datetime_t date = {
            .year  = 2024,
            .month = 06,
            .day   = 20,
            .dotw  = 05,
            .hour  = 00,
            .min   = 00,
            .sec   = 00
    };    

    initializeRTC(date);
    sleep_us(64);

    // Initialize chosen serial port
    stdio_init_all();

    // Initialize SD card
    if (!sd_init_driver()) {
        printf("ERROR: Could not initialize SD card\r\n");
        while (true);
    }

    /* FILE SYSTEM INITIALIZATION */
    // Mount drive
    fileResult = f_mount(&fileSystem0, "0:", 1);
    fileResult = f_mount(&fileSystem1, "1:", 1);

    // Open file for writing
    fileResult = f_open(&file0, filename0, FA_WRITE | FA_CREATE_ALWAYS);
    fileResult = f_open(&file1, filename1, FA_WRITE | FA_CREATE_ALWAYS);

    // write column headings
    ret = f_printf(&file0, "time_diff (us), MFC_C (V), MFC_E (V)\n");
    ret = f_printf(&file1, "time_diff (us), MFC_C (V), MFC_E (V)\n");
    
    
    int64_t write_time = 0;
    
    while (!core0_finished){
        if (bytes_in_buffer() >= 512) {
            for (int i = 0; i < 512; i++) {
                uint16_t mfc_control;
                uint16_t mfc_experimental;
                int64_t time_dif;

                // Remove data from the circular buffer
                if (removeFromBuffer(&mfc_control, &mfc_experimental, &time_dif)) {
                    // Data successfully removed from buffer
                    char buffer[100];
                    sprintf(buffer, "%09d,%0.4f,%0.4f\n", time_dif, mfc_control * CONVERSION_FACTOR, mfc_experimental * CONVERSION_FACTOR);
                    f_write(&file0, buffer, strlen(buffer), NULL);
                    f_write(&file1, buffer, strlen(buffer), NULL);
                }
            }
        }
    }
    
    // Close file
    fileResult = f_close(&file0);
    fileResult = f_close(&file1);

    // Unmount drive
    f_unmount("0:");
    f_unmount("1:");
}

// Setup up method for Core 0
void core_0(){
    /* RTC INITIALIZATION */
    data_buffer.write_index = 0;
    data_buffer.read_index = 0;
    stdio_init_all();
    
    multicore_reset_core1();

    multicore_launch_core1(core_1);

    char datetime_buf[256];
    char *datetime_str = &datetime_buf[0];

    // Start at 0 for H:M:S
    datetime_t date = {
            .year  = 2024,
            .month = 06,
            .day   = 20,
            .dotw  = 05,
            .hour  = 00,
            .min   = 00,
            .sec   = 00
    };

    /* INITIALIZE PERIPHIALS */
    initalizeADC(ADC_PIN_0, ADC_CHANNEL_0); // mfc control patch
    initalizeADC(ADC_PIN_1, ADC_CHANNEL_1); // mfc experimental patch 
    initializeI2C();                        // I2C
    initializeGPIO(SOLENOID_PIN, GPIO_OUT); // Solenoid
    initializeGPIO(BUZZER_PIN, GPIO_OUT);   // Buzzer

    // gpio_put(BUZZER_PIN,1);
    // sleep_ms(5000);
    // gpio_put(BUZZER_PIN,0);

    uint16_t mfc_control = 0;
    uint16_t mfc_experimental = 0;
    int counter = 0;
    char solenoidSet = 0;

    initializeRTC(date);
    sleep_us(64);

    gpio_put(BUZZER_PIN, 1);
    sleep_ms(100);
    gpio_put(BUZZER_PIN, 0);
    sleep_ms(100);
    gpio_put(BUZZER_PIN, 1);
    sleep_ms(100);
    gpio_put(BUZZER_PIN, 0);

    gpio_put(BUZZER_PIN,0); // resets buzzer
    double acc_z = (-1 * bnoReadZ()) / 100.0;

    absolute_time_t startTime = get_absolute_time();
    absolute_time_t prevTime = get_absolute_time();
    int64_t time_dif = 0;

    while (time_dif < 10000000) { // launch will last 300 seconds
        prevTime = get_absolute_time();
        if (time_dif >= 5000000 && time_dif < 7000000 && !solenoidSet) {
            gpio_put(SOLENOID_PIN, 1);
            solenoidSet = 1;
        } // if
        else if (time_dif >= 7000000 && time_dif < 9000000 && solenoidSet) {
            gpio_put(SOLENOID_PIN, 0);
        } // else if

        adc_select_input(0);
        mfc_control = adc_read();
        adc_select_input(1);
        mfc_experimental = adc_read();
        time_dif = absolute_time_diff_us(startTime, get_absolute_time());

        addToBuffer(mfc_control, mfc_experimental, time_dif);
        
        counter += 1;

        while ((absolute_time_diff_us(prevTime, get_absolute_time())) < 800) {} 
        //800 seems to be the lowest before running to issues. This produces a 1250 Hz frequency

    
    } // while
    core0_finished = true;
    gpio_put(BUZZER_PIN, 1);
    sleep_ms(100);
    gpio_put(BUZZER_PIN, 0);
    sleep_ms(100);
    gpio_put(BUZZER_PIN, 1);
    sleep_ms(100);
    gpio_put(BUZZER_PIN, 0);

}


/*
* Entry point into program
*/
int main() {
    initializeCircularBuffer();
    core_0();
    return 0;
}