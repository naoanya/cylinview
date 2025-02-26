/**********************************************************************/
/**
 * @brief  Monochrome (8bit Packed) Screen
 * @author naoa
 */
/**********************************************************************/
#pragma once
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Include files
 *----------------------------------------------------------------------
 */
#include <cstdint>
#include "screen_base.hpp"
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

class MonoScreen : public ScreenBase<color_t>
{
public:
    explicit MonoScreen();
    virtual ~MonoScreen();

public:
    int     width(void) override { return CV_HEIGHT; }
    int     height(void) override { return CV_WIDTH; }
    int     pixels(void) override { return CV_PIXELS; }
    color_t getClearColor(void) override { return DISP_COLOR_BLACK; }
    void    clear(color_t c = DISP_COLOR_BLACK) override;
    void    setDot(int x, int y, color_t c) override;
    color_t getDot(int x, int y) override;

public:
    void      setBuffer(color_t * buffer);
    color_t * getBuffer(void);

private:
    color_t * buffer_;
};
