/**********************************************************************/
/**
 * @brief  DRV8833 Motor Driver
 * @author naoa
 */
/**********************************************************************/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Include files
 *----------------------------------------------------------------------
 */
#include <cstdint>
#include <Arduino.h>

#include "motor.hpp"

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Definitions
 *----------------------------------------------------------------------
 */

static int pin_in_1_;
static int pin_in_2_;
static int decay_mode_ = 0;

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Function definitions
 *----------------------------------------------------------------------
 */

void motor_init(
    int pin_in_1,
    int pin_in_2
) {
    pin_in_1_ = pin_in_1;
    pin_in_2_ = pin_in_2;
    pinMode( pin_in_1_, OUTPUT );
    pinMode( pin_in_2_, OUTPUT );
}

void motor_set_power(
    int power
) {
    if (power > 0) {
        analogWrite(pin_in_1_, power);
        analogWrite(pin_in_2_, decay_mode_);
    } else if (power < 0) {
        analogWrite(pin_in_1_, decay_mode_);
        analogWrite(pin_in_2_, -power);
    } else {
        analogWrite(pin_in_1_, decay_mode_);
        analogWrite(pin_in_2_, decay_mode_);
    }
}

void motor_set_brake(
    bool brake
) {
    int v = (brake)? 255 : 0;
    analogWrite(pin_in_1_, v);
    analogWrite(pin_in_2_, v);
}

void motor_set_decay_mode(
    bool slow
) {
    decay_mode_ = (slow)? 1 : 0;
}
