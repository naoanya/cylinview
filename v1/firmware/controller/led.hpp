/**********************************************************************/
/**
 * @brief  LED
 * @author naoa
 */
/**********************************************************************/
#pragma once
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Include files
 *----------------------------------------------------------------------
 */

#include <Arduino.h>

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Class definitions
 *----------------------------------------------------------------------
 */

class LED
{
public:

    LED() :
        pinLed_(0),
        blinkIntervalMs_(0)
    {
        /* init at setup() */
    }

    LED(
        int pinLed,
        unsigned long blinkIntervalMs
    ) :
        pinLed_(pinLed),
        blinkIntervalMs_(blinkIntervalMs)
    {
        pinMode(pinLed_, OUTPUT);
    }

    void setup(
        int pinLed,
        unsigned long blinkIntervalMs
    ) {
        pinLed_ = pinLed;
        blinkIntervalMs_ = blinkIntervalMs;
        pinMode(pinLed_, OUTPUT);
    }

    void loop(void)
    {
        if (!enable_) return;
        unsigned long curMillis = millis();
        if (curMillis - preMillis_ >= 500) {
            preMillis_ = curMillis;
            if (ledDir_) {
                digitalWrite(pinLed_, LOW);
                ledDir_ = false;
            } else {
                digitalWrite(pinLed_, HIGH);
                ledDir_ = true;
            }
        }
    }

    void on(bool on = true) { enable_ = true; }
    void off(void) { enable_ = false; }

private:
    int pinLed_;
    unsigned long blinkIntervalMs_ = 500;

private:
    bool ledDir_ = false;
    unsigned long preMillis_ = 0;

    bool enable_ = true;
};
