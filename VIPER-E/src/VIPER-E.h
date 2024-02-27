#include <stdio.h>
#include "pico/stdlib.h"

#include "sd_card.h"
#include "ff.h"

#include "hardware/gpio.h"
#include "hardware/adc.h"
#include "hardware/i2c.h"
#include "hardware/rtc.h"
#include "pico/util/datetime.h"

#include "pico/binary_info.h"

#define BNO055_ADDRESS 0x28

// Register Addresses
#define CONFIGURATION_REGISTER 0x3D
#define UNIT_REGISTER 0x3B
#define ACCEL_Z_AXIS 0xC

// Common Hex Values
#define IMU_MODE 0x04
#define UNITS 0x00