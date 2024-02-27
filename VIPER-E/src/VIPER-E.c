#include "VIPER-E.h"

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
    // Select ADC input 0 (GPIO26)
    adc_gpio_init(GPIO);
    adc_select_input(channel);
} // initializeADC

/*
* Initializes I2C communications for the BNO055 Sensor. Uses the default 
* SDA and SCL pins on the pico (GPIO 4 (pin 6) and 5 (pin 7)) with a baud
* rate of 400000.
*/
void initializeI2C() {
    i2c_init(i2c_default, 400 * 1000);
    gpio_set_function(PICO_DEFAULT_I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(PICO_DEFAULT_I2C_SDA_PIN);
    gpio_pull_up(PICO_DEFAULT_I2C_SCL_PIN);
    
    // Make the I2C pins available to picotool
    bi_decl(bi_2pins_with_func(PICO_DEFAULT_I2C_SDA_PIN, PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C));

    // reset the BNO055
    bnoReset();
} // initializeI2C

/*
* Entry point into program
*/
int main() {
    // variables to access and write to file
    FRESULT fileResult;
    FATFS fileSystem;
    FIL file;
    int ret;
    char buf[100];
    char filename[] = "prototype.csv";

    // Initialize SD card
    if (!sd_init_driver()) {
        printf("ERROR: Could not initialize SD card\r\n");
        while (true);
    }

    // Mount drive
    fileResult = f_mount(&fileSystem, "0:", 1);
    if (fileResult != FR_OK) {
        printf("ERROR: Could not mount filesystem (%d)\r\n", fileResult);
        while (true);
    }

    // Open file for writing ()
    fileResult = f_open(&file, filename, FA_WRITE | FA_CREATE_ALWAYS);
    if (fileResult != FR_OK) {
        printf("ERROR: Could not open file (%d)\r\n", fileResult);
        while (true);
    }

    double acc_z = 0;

    char datetime_buf[256];
    char *datetime_str = &datetime_buf[0];

    // Start on Friday 5th of June 2020 15:45:00
    datetime_t date = {
            .year  = 2024,
            .month = 02,
            .day   = 20,
            .dotw  = 2, // 0 is Sunday, so 5 is Friday
            .hour  = 18,
            .min   = 00,
            .sec   = 00
    };

    // Initialize chosen serial port
    stdio_init_all();

    // Initialize periphials
    initalizeADC(26, 0);
    initializeI2C();
    initializeRTC(date);
    sleep_us(64);

    // Wait for user to press 'enter' to continue
    printf("\r\nAnalog read test. Press 'enter' to start.\r\n");
    while (true) {
        buf[0] = getchar();
        if ((buf[0] == '\r') || (buf[0] == '\n')) {
            break;
        }
    }

    int min = date.min;
    uint16_t result = 0;
    const float conversion_factor = 3.3f / (1 << 12);
    int counter = 0;

    // Close file
    fileResult = f_close(&file);
    if (fileResult != FR_OK) {
        printf("ERROR: Could not close file (%d)\r\n", fileResult);
        while (true);
    }

    // Unmount drive
    f_unmount("0:");

    // // Loop forever doing nothing
    while (true) {
        sleep_ms(1000);
    }

}