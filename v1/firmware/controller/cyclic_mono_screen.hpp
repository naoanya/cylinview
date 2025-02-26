/**********************************************************************/
/**
 * @brief  Cyclic Monochrome (8bit Packed) Screen
 * @author naoa
 */
/**********************************************************************/
#pragma once
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Include files
 *----------------------------------------------------------------------
 */
#include <cstdint>
#include <cstdbool>

#include "screen_base.hpp"
#include "screen_config.hpp"
#include "mono_screen.hpp"

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Definitions
 *----------------------------------------------------------------------
 */

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Definitions
 *----------------------------------------------------------------------
 */

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

class CyclicMonoScreen : public ScreenBase<bool>
{
public:
    explicit CyclicMonoScreen();
    virtual ~CyclicMonoScreen();

public:
    int width(void) override { return CV_V_WIDTH; }
    int height(void) override { return CV_HEIGHT; }
    int pixels(void) override { return CV_V_PIXELS; }
    bool getClearColor(void) override { return false; }
    void clear(bool c = 0) override;
    void setDot(int x, int y, bool c) override;
    bool getDot(int x, int y) override;

public:
    MonoScreen * getMonoScreen(int index);

public:
    MonoScreen screens_[CV_DISPLAYS];
};
