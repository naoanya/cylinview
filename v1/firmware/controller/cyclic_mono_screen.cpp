/**********************************************************************/
/**
 * @brief  Cyclic Monochrome (8bit Packed) Screen
 * @author naoa
 */
/**********************************************************************/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Include files
 *----------------------------------------------------------------------
 */
#include "cyclic_mono_screen.hpp"

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Definitions - Debug
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
 * Definitions
 *----------------------------------------------------------------------
 */

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Function definitions
 *----------------------------------------------------------------------
 */

CyclicMonoScreen::CyclicMonoScreen()
{
}

CyclicMonoScreen::~CyclicMonoScreen()
{
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Function definitions
 *----------------------------------------------------------------------
 */

void
CyclicMonoScreen::clear(bool c)
{
    for (int i = 0; i < CV_DISPLAYS; i++) {
        screens_[i].clear(c);
    }
}

bool
CyclicMonoScreen::getDot(int x, int y)
{
    x = -x;
    x -= (CV_MARGIN + 1);

    if (y < 0 || CV_HEIGHT <= y) return 0;

    while (x < 0) {
        x += CV_V_WIDTH;
    }

    // get "offset x" per screen.
    int screens_x = x % (CV_WIDTH + CV_MARGIN);
    if (screens_x >= CV_WIDTH) {
        // margin area
        return 0;
    }
    
    // get screen number.
    int screens_i = x / (CV_WIDTH + CV_MARGIN);

    // convert overflowed number to continous screen number.
    screens_i = screens_i % CV_DISPLAYS;

    return screens_[screens_i].getDot(screens_x, y);
}

void
CyclicMonoScreen::setDot(int x, int y, bool c)
{
    x = -x;
    x -= (CV_MARGIN + 1);
    
    if (y < 0 || CV_HEIGHT <= y) return;

    while (x < 0) {
        x += CV_V_WIDTH;
    }

    // get "offset x" per screen.
    int screens_x = x % (CV_WIDTH + CV_MARGIN);
    if (screens_x >= CV_WIDTH) {
        // margin area
        return;
    }

    // get screen number.
    int screens_i = x / (CV_WIDTH + CV_MARGIN);

    // convert overflowed number to continous screen number.
    screens_i = screens_i % CV_DISPLAYS;

    screens_[screens_i].setDot(screens_x, y, c);
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Function definitions
 *----------------------------------------------------------------------
 */

MonoScreen *
CyclicMonoScreen::getMonoScreen(int index)
{
    return &screens_[index];
}
