/**********************************************************************/
/**
 * @brief  AS5048A Magnetic Encoder
 * 
 *  Reference: Arduino-FOC/src/sensors/MagneticSensorPWM.cpp
 * 
 * @author naoa
 */
/**********************************************************************/
#pragma once
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Include files
 *----------------------------------------------------------------------
 */
#include <Arduino.h>
#include <cstdint>

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Definitions
 *----------------------------------------------------------------------
 */

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Class definitions
 *----------------------------------------------------------------------
 */

void encoder_init_pwm(
    int pin_pwm
);
float encoder_get_angle_pwm(void);
uint32_t encoder_get_raw_data_pwm(void);

void encoder_init_spi(
    int pin_rx,
    int pin_cs,
    int pin_sck,
    int pin_tx
);
float encoder_get_angle_spi(void);
uint32_t encoder_get_raw_data_spi(void);
void encoder_set_angle_offset(uint16_t offset);
