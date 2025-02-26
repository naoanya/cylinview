/**********************************************************************/
/**
 * @brief  Easy Drawer for Cyclic Monochrome (8bit Packed) Screen
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
#include <functional>

#include "screen_config.hpp"
#include "cyclic_mono_screen.hpp"
#include "mono_image.hpp"

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

class CyclicMonoDrawer
{
public:
    explicit CyclicMonoDrawer();
    virtual ~CyclicMonoDrawer();

public:
    void init(CyclicMonoScreen * screen);

public:
    int         width(void) { return width_; }
    int         height(void) { return height_; }

public:
    void        clearFrame(color_t color = DISP_COLOR_BLACK);
    color_t     getDot(int x, int y) const;
    void        setDot(int x, int y, color_t c = DISP_COLOR_WHITE);
    int         drawDot(int x, int y, color_t c = DISP_COLOR_WHITE);
    int         drawHLine(int x1, int x2, int y, color_t c = DISP_COLOR_WHITE);
    int         drawVLine(int x, int y1, int y2, color_t c = DISP_COLOR_WHITE);
    int         drawLine(int x1, int y1, int x2, int y2, color_t c = DISP_COLOR_WHITE);
    int         drawRect(int x1, int y1, int x2, int y2, color_t c = DISP_COLOR_WHITE, bool fill = false);
    int         drawRectNoFill(int x1, int y1, int x2, int y2, color_t c = DISP_COLOR_WHITE);
    int         drawRectFill(int x1, int y1, int x2, int y2, color_t c = DISP_COLOR_WHITE);
    int         drawTriangleFillScanLine(double& l_x, double& l_a, double& r_x, double& r_a, int& sy, int ey, color_t c = DISP_COLOR_WHITE);
    int         drawTriangleFill(int x1, int y1, int x2, int y2, int x3, int y3, color_t c = DISP_COLOR_WHITE);
    int         drawTriangle(int x1, int y1, int x2, int y2, int x3, int y3, color_t c = DISP_COLOR_WHITE);
    void        drawCircle(int x0, int y0, int radius, color_t c = DISP_COLOR_WHITE);
    void        drawCircleFill(int x0, int y0, int radius, color_t c = DISP_COLOR_WHITE);

public:
    void        drawImage(int x, int y, MonoImage * image, bool blend = false, bool centered = false, bool offset = false);
    void        drawImageCentered(int x, int y, MonoImage * image);
    void        drawImageBlend(int x, int y, MonoImage * image);
    void        drawImageBlendCentered(int x, int y, MonoImage * image);
    void        drawImageOffset(int x, int y, MonoImage * image);
    void        drawImageOffsetCentered(int x, int y, MonoImage * image);
    void        drawImageBlendOffset(int x, int y, MonoImage * image);
    void        drawImageBlendOffsetCentered(int x, int y, MonoImage * image);

private:
    int width_;
    int height_;
    int pixels_;

public:
    CyclicMonoScreen * screen_;
};
