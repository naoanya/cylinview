/**********************************************************************/
/**
 * @brief  Falling Snow Maker
 * @author naoa
 */
/**********************************************************************/
#pragma once
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Include files
 *----------------------------------------------------------------------
 */

#include <cstdbool>
#include <cstdint>

#include "screen_config.hpp"

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Definitions
 *----------------------------------------------------------------------
 */

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Class Forword Declarations
 *----------------------------------------------------------------------
 */

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Name Space
 *----------------------------------------------------------------------
 */

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Definitions
 *----------------------------------------------------------------------
 */

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Class definitions
 *----------------------------------------------------------------------
 */

class Snow
{
public:
    typedef uint32_t (*RandCallback)(void);

    class Grain
    {
    public:
        int x_ = 0;
        int y_ = CV_HEIGHT;
    };

public:
    int fallSpeed_ = 1;
    int shakeRange_ = 3;
    int shakeOffset_ = -1;
    RandCallback rnd_;
    Grain * grains_;
    int count_;

public:
    void setSnowGrains(Grain * grains, int count) {
        grains_ = grains;
        count_ = count;
    }
    void setRandCallback(RandCallback rnd) {
        rnd_ = rnd;
    }
    void setFallSpeed(int speed) {
        fallSpeed_ = speed;
    }
    void setShakeRange(int range) {
        shakeRange_ = range;
    }
    void setShakeOffset(int offset) {
        shakeOffset_ = offset;
    }
    int getFallSpeed(void) {
        return fallSpeed_;
    }
    int getShakeRange(void) {
        return shakeRange_;
    }
    int getShakeOffset(void) {
        return shakeOffset_;
    }

public:
    void loop() {
        for (int i = 0; i < count_; i++) {
            Grain * grain = &grains_[i];
            if (grain->y_ >= CV_HEIGHT) {
                // on grounded. reset to start positon.
                uint32_t rnd = rnd_(); // reduce rnd call.
                uint16_t rnd1 = rnd >> 16;
                uint16_t rnd2 = rnd >>  0;
                grain->x_ = rnd1 % CV_V_WIDTH;
                grain->y_ = -(rnd2 % CV_HEIGHT);
            } else {
                grain->x_ += ((rnd_() % shakeRange_) + shakeOffset_);
                grain->y_ += fallSpeed_;
            }
        }
    }
};
