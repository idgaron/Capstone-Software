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

/*
* Entry point into program
*/
int main() {
    // variables to access and write to file
    FRESULT fileResult;
    FATFS fileSystem0, fileSystem1;
    FIL file0, file1;
    int ret;
    char buf[100];
    char filename0[] = "0:/TEST_LAUNCH_SD0.csv";
    char filename1[] = "1:/TEST_LAUNCH_SD1.csv";

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

    /* RTC INITIALIZATION */
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

    /* TESTING PROMPT - REMOVE BEFORE USE */
    // Wait for user to press 'enter' to continue
    // printf("\nPrototype test. Press 'enter' to start.\n");
    // while (true) {
    //     buf[0] = getchar();
    //     if ((buf[0] == '\r') || (buf[0] == '\n')) {
    //         break;
    //     }
    // }

    gpio_put(BUZZER_PIN,1);
    sleep_ms(5000);
    gpio_put(BUZZER_PIN,0);

    uint16_t mfc_control = 0;
    uint16_t mfc_experimental = 0;
    int counter = 0;
    char solenoidSet = 0;

    initializeRTC(date);
    sleep_us(64);

    // waits until positive acceleration to start logging data
    //while (((-1 * bnoReadZ()) / 100.0) < 0) {
    while ((bnoReadZ()/100.0 < 24.53) && (bnoReadZ()/100.0 > -24.53)) {
        //printf("%0.2f\n", ((-1 * bnoReadZ()) / 100.0));
        if (date.sec % 10 == 0) {
            gpio_put(BUZZER_PIN,1);
        }
        else {
            gpio_put(BUZZER_PIN,0);
        }
        rtc_get_datetime(&date);
    }

    gpio_put(BUZZER_PIN,0); // resets buzzer
    double acc_z = (-1 * bnoReadZ()) / 100.0;

    absolute_time_t startTime = get_absolute_time();
    absolute_time_t prevTime = get_absolute_time();
    int64_t time_dif = 0;

    while (time_dif < 300000000) { // launch will last 300 seconds
        prevTime = get_absolute_time();
        if (time_dif >= 500000 && time_dif < 1500000 && !solenoidSet) {
            gpio_put(SOLENOID_PIN, 1);
            solenoidSet = 1;
        } // if
        else if (time_dif >= 1500000 && time_dif < 3000000 && solenoidSet) {
            gpio_put(SOLENOID_PIN, 0);
        } // else if

        adc_select_input(0);
        mfc_control = adc_read();
        adc_select_input(1);
        mfc_experimental = adc_read();

        // // terminal output - comment out for actual implementation
        printf("time: %d (x100us), voltage: %f V, acceleration: %0.2f m/s^2\n", counter, mfc_control * CONVERSION_FACTOR, acc_z);

        time_dif = absolute_time_diff_us(startTime, get_absolute_time());
        // ret = f_printf(&file0, "%09d,%0.4f,%0.4f\n", time_dif, mfc_control * CONVERSION_FACTOR, mfc_experimental * CONVERSION_FACTOR);
        // ret = f_printf(&file1, "%09d,%0.4f,%0.4f\n", time_dif, mfc_control * CONVERSION_FACTOR, mfc_experimental * CONVERSION_FACTOR);

        counter += 1;
        while ((absolute_time_diff_us(prevTime, get_absolute_time())) < 500) {}
    } // while
    
    // Close file
    fileResult = f_close(&file0);
    fileResult = f_close(&file1);

    // Unmount drive
    f_unmount("0:");
    f_unmount("1:");

}