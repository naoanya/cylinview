/**********************************************************************/
/**
 * @brief  Character
 * @author naoa
 */
/**********************************************************************/
#pragma once
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Include files
 *----------------------------------------------------------------------
 */
#include <Arduino.h>

#include <cstdbool>
#include <cstdint>

#include "screen_config.hpp"
#include "image_data.hpp"

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

class Character
{
public:
    typedef uint32_t (*RandCallback)(void);

public:
    RandCallback rnd_;

    int state_;
    int nextState_;
    
    const mono_images_t * curFrames_;
    int curFrameIndex_;
    int move_;
    int xpos_;
    
    int skip_;

    bool auto_;
    int autoTime_;

public:

    void init(void)
    {
        state_ = 0;
        nextState_ = 0;
        curFrameIndex_ = 0;
        move_ = 0;
        xpos_ = 0;
        skip_ = 0;
        auto_ = false;
        autoTime_ = 0;

        setNextState(-1, true);
    }

    void setRandCallback(RandCallback rnd)
    {
        rnd_ = rnd;
    }

    void setAuto(bool enable = true)
    {
        auto_ = enable;
        autoTime_ = rnd_() % (70 * 4);
    }

public:
    void setNextState(int state, bool immediate = false)
    {
        if (nextState_ != -1 && immediate) {
            setCurState(nextState_);
            curFrameIndex_ = 0;
            nextState_ = -1;
        } else if (state != -1) {
            nextState_ = state;
        }
    }

public:
    void setCurState(int state)
    {
        state_ = state;

        switch (state_)
        {
        case 0:  curFrames_ = &image_idle2_frames; move_ = 0; break;
        case 1:  curFrames_ = &image_run1_frames;  move_ = 2; break;
        case 2:  curFrames_ = &image_jump1_frames; move_ = 2; break;
        default: curFrames_ = &image_idle2_frames; state_ = 0; break;
        }
    }

public:
    int getXpos(void)
    {
        return xpos_;
    }

    const mono_image_t * getImage(void)
    {
        const mono_image_t * image = &curFrames_->images_[curFrameIndex_];

        if (auto_) {
            autoTime_--;
            if (autoTime_ <= 0) {
                setNextState((int)(rnd_() % 3));
                setAuto();
            }
        }

        // @todo frame countup calculation from fps.
        skip_++;
        if (skip_ > 2) {
            skip_ = 0;

            xpos_ += move_;

            curFrameIndex_++;
            if (curFrameIndex_ >= curFrames_->count_) {
                curFrameIndex_ = 0;
                setNextState(-1, true);
            }
        }

        return image;
    }
};
