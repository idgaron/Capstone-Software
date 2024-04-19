#include <stdio.h>
#include "pico/stdlib.h"

#include "sd_card.h"
#include "ff.h"

#include "hardware/gpio.h"
#include "hardware/adc.h"
#include "hardware/i2c.h"
#include "hardware/rtc.h"
#include "pico/util/datetime.h"
#include "pico/multicore.h"

#include "pico/binary_info.h"

#define BNO055_ADDRESS 0x28

// Register Addresses
#define CONFIGURATION_REGISTER 0x3D
#define UNIT_REGISTER 0x3B
#define ACCEL_Z_AXIS 0xC

// Common Hex Values
#define IMU_MODE 0x01
#define UNITS 0x00

// GPIOs
#define ADC_PIN_0 26
#define ADC_PIN_1 27
#define ADC_CHANNEL_0 0
#define ADC_CHANNEL_1 1
#define SOLENOID_PIN 6
#define BUZZER_PIN 15

#define CONVERSION_FACTOR 3.3f / (1 << 12)