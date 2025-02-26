/**********************************************************************/
/**
 * @brief  Easy Drawer for Cyclic Monochrome (8bit Packed) Screen
 * @author naoa
 */
/**********************************************************************/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Include files
 *----------------------------------------------------------------------
 */
#include "cyclic_mono_drawer.hpp"

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Definitions - Debug
 *----------------------------------------------------------------------
 */

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Definitions
 *----------------------------------------------------------------------
 */

#ifndef UNUSED_VAR
#define UNUSED_VAR(x)   ((void)x)
#endif

#ifndef ABS
#define ABS(x)          (((x) >= 0)? (x) : -(x))
#endif

#ifndef MAX
#define MAX(x,y)        (((x) >= (y))? (x) : (y))
#endif

#ifndef MIN
#define MIN(x,y)        (((x) <= (y))? (x) : (y))
#endif

#ifndef SIGNUM
#define SIGNUM(x)       (((x) > 0)? (1) : ((x) < 0)? (-1) : (0))
#endif

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

CyclicMonoDrawer::CyclicMonoDrawer()
{
}

CyclicMonoDrawer::~CyclicMonoDrawer()
{
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Function definitions
 *----------------------------------------------------------------------
 */

void
CyclicMonoDrawer::init(
    CyclicMonoScreen * screen
) {
    screen_ = screen;

    width_ = screen_->width();
    height_ = screen_->height();
    pixels_ = width_ * height_;
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Method definitions
 *----------------------------------------------------------------------
 */

void
CyclicMonoDrawer::clearFrame(color_t c)
{
    screen_->clear(c);
}

color_t
CyclicMonoDrawer::getDot(int x, int y) const
{
    return screen_->getDot(x, y);
}

void
CyclicMonoDrawer::setDot(int x, int y, color_t c)
{
    screen_->setDot(x, y, c);
}

int
CyclicMonoDrawer::drawDot(int x, int y, color_t c)
{
    setDot(x, y, c);
    return 0;
}

int
CyclicMonoDrawer::drawHLine(int x1, int x2, int y, color_t c)
{
    if (y < 0 || height_ <= y) return -1;
    /*DisableForCyclic*///if ((x1 < 0 && x2 < 0) || (width_ <= x1 && width_ <= x2)) return -1;

    if (x1 > x2) std::swap(x1, x2);
    
    /*DisableForCyclic*///if (x1 < 0) x1 = 0;
    /*DisableForCyclic*///if (x2 >= width_) x2 = width_ - 1;

    int l = x2 - x1 + 1;
    for (int i = 0; i < l; i++) {
        drawDot(x1 + i, y, c);
    }
    return 0;
}

int
CyclicMonoDrawer::drawVLine(int x, int y1, int y2, color_t c)
{
    /*DisableForCyclic*///if (x < 0 || width_ <= x) return -1;
    if ((y1 < 0 && y2 < 0) || (height_ <= y1 && height_ <= y2)) return -1;

    if (y1 > y2) std::swap(y1, y2);

    if (y1 < 0) y1 = 0;
    if (y2 >= height_) y2 = height_ - 1;

    int l = y2 - y1 + 1;
    for (int i = 0; i < l; i++) {
        drawDot(x , y1 + i, c);
    }
    return 0;
}

int
CyclicMonoDrawer::drawLine(int x1, int y1, int x2, int y2, color_t c)
{
    int xinc1 = 0, xinc2 = 0;
    int yinc1 = 0, yinc2 = 0;
    if (x2 >= x1)   { xinc1 =  1;   xinc2 =  1;}
    else            { xinc1 = -1;   xinc2 = -1;}
    if (y2 >= y1)   { yinc1 =  1;   yinc2 =  1;}
    else            { yinc1 = -1;   yinc2 = -1;}

    int den = 0;
    int num = 0;
    int numadd = 0;
    int numpixels = 0;
    int deltax = ABS(x2 - x1);
    int deltay = ABS(y2 - y1);
    if (deltax >= deltay) {
        xinc1 = 0;
        yinc2 = 0;
        den = deltax;
        num = deltax / 2;
        numadd = deltay;
        numpixels = deltax;
    } else {
        xinc2 = 0;
        yinc1 = 0;
        den = deltay;
        num = deltay / 2;
        numadd = deltax;
        numpixels = deltay;
    }

    int x = x1;
    int y = y1;
    for (int curpixel = 0; curpixel <= numpixels; curpixel++) {
        drawDot(x, y, c);
        num += numadd;
        if (num >= den) {
            num -= den;
            x += xinc1; y += yinc1;
        }
        x += xinc2; y += yinc2;
    }

    return 0;
}

int
CyclicMonoDrawer::drawRect(int x1, int y1, int x2, int y2, color_t c, bool fill)
{
    return (fill)? drawRectFill(x1,y1,x2,y2,c) : drawRectNoFill(x1,y1,x2,y2,c);
}

int
CyclicMonoDrawer::drawRectNoFill(int x1, int y1, int x2, int y2, color_t c)
{
    /*DisableForCyclic*///if ((x1 < 0 && x2 < 0) || (width_ <= x1 && width_ <= x2)) return -1;
    /*DisableForCyclic*///if ((y1 < 0 && y2 < 0) || (height_ <= y1 && height_ <= y2)) return -1;
    drawHLine(x1, x2, y1, c);
    drawHLine(x1, x2, y2, c);
    drawVLine(x1, y1, y2, c);
    drawVLine(x2, y1, y2, c);
    return 0;
}

int
CyclicMonoDrawer::drawRectFill(int x1, int y1, int x2, int y2, color_t c)
{
    /*DisableForCyclic*///if ((x1 < 0 && x2 < 0) || (width_ <= x1 && width_ <= x2)) return -1;
    /*DisableForCyclic*///if ((y1 < 0 && y2 < 0) || (height_ <= y1 && height_ <= y2)) return -1;

    if (y1 > y2) std::swap(y1, y2);

    for (int i = y1; i <= y2; i++) drawHLine(x1, x2, i, c);
    return 0;
}

int
CyclicMonoDrawer::drawTriangleFillScanLine(
        double& l_x, double& l_a, double& r_x, double& r_a,
        int& sy, int ey, color_t c )
{
    int width_m1 = width_ - 1;
    for ( ; sy < ey ; ++sy ) {
        int sx = (int)(l_x + 0.5);
        int ex = (int)(r_x + 0.5);
        sx = (l_x < 0)? 0 : sx;
        if ( ex > width_m1 ) ex = width_m1;
        drawHLine(sx, ex, sy, c);
        l_x += l_a; r_x += r_a;
    }
    return 0;
}

int
CyclicMonoDrawer::drawTriangleFill(int x1, int y1, int x2, int y2, int x3, int y3, color_t c)
{
    if ( y1 > y2 ) { std::swap(x1, x2); std::swap(y1, y2); }
    if ( y1 > y3 ) { std::swap(x1, x3); std::swap(y1, y3); }
    if ( y2 > y3 ) { std::swap(x2, x3); std::swap(y2, y3); }
    int top_x = x1, top_y = y1;
    int mid_x = x2, mid_y = y2;
    int btm_x = x3, btm_y = y3;

    if ( top_y >= height_ ) return -1;
    if ( btm_y < 0 ) return -1;
    /*DisableForCyclic*///if ( x1 < 0 && x2 < 0 && x3 < 0 ) return -1;
    /*DisableForCyclic*///if ( x1 >= width_ && x2 >= width_ && x3 >= width_ ) return -1;

    double top_mid_x = top_x;
    double top_btm_x = top_x;

    if ( top_y == mid_y ) top_mid_x = mid_x;

    int sy = top_y;
    int my = mid_y;
    int ey = btm_y;

    if ( top_y < 0 ) {
        sy = 0;
        if ( mid_y >= 0 ) {
            if ( top_y != mid_y )
                top_mid_x = (double)( mid_x - top_x ) * (double)mid_y / (double)( top_y - mid_y ) + (double)mid_x;
        } else {
            if ( mid_y != btm_y )
                top_mid_x = (double)( btm_x - mid_x ) * (double)btm_y / (double)( mid_y - btm_y ) + (double)btm_x;
        }
        if ( top_y != btm_y )
            top_btm_x = (double)( btm_x - top_x ) * (double)btm_y / (double)( top_y - btm_y ) + (double)btm_x;
    }

    if ( btm_y >= height_ ) ey = height_ - 1;

    double top_mid_a = ( mid_y != top_y ) ?
      (double)( mid_x - top_x ) / (double)( mid_y - top_y ) : 0;
    double mid_btm_a = ( mid_y != btm_y ) ?
      (double)( mid_x - btm_x ) / (double)( mid_y - btm_y ) : 0;
    double top_btm_a = ( top_y != btm_y ) ?
      (double)( top_x - btm_x ) / (double)( top_y - btm_y ) : 0;

    int splitLine_x = ( top_y != btm_y ) ?
      ( top_x - btm_x ) * ( mid_y - top_y ) / ( top_y - btm_y ) + top_x :
      btm_x;

    double l_x, l_a, r_x, r_a;
    if ( mid_x < splitLine_x) {
        l_x = top_mid_x;
        l_a = top_mid_a;
        r_x = top_btm_x;
        r_a = top_btm_a;
    } else {
        l_x = top_btm_x;
        l_a = top_btm_a;
        r_x = top_mid_x;
        r_a = top_mid_a;
    }

    drawTriangleFillScanLine( l_x, l_a, r_x, r_a , sy, my, c );
    if ( mid_x < splitLine_x) {
        l_a = mid_btm_a;
    } else {
        r_a = mid_btm_a;
    }
    drawTriangleFillScanLine( l_x, l_a, r_x, r_a , sy, ey + 1, c);

    return 0;
}

int
CyclicMonoDrawer::drawTriangle(int x1, int y1, int x2, int y2, int x3, int y3, color_t c)
{
    drawLine(x1, y1, x2, y2, c);
    drawLine(x1, y1, x3, y3, c);
    drawLine(x2, y2, x3, y3, c);
    return 0;
}

void
CyclicMonoDrawer::drawCircle(int x0, int y0, int radius, color_t c)
{
    int x = radius-1;
    int y = 0;
    int dx = 1;
    int dy = 1;
    int err = dx - (radius << 1);

    while (x >= y) {
        drawDot(x0 + x, y0 + y, c);
        drawDot(x0 + y, y0 + x, c);
        drawDot(x0 - y, y0 + x, c);
        drawDot(x0 - x, y0 + y, c);
        drawDot(x0 - x, y0 - y, c);
        drawDot(x0 - y, y0 - x, c);
        drawDot(x0 + y, y0 - x, c);
        drawDot(x0 + x, y0 - y, c);

        if (err <= 0) {
            y++;
            err += dy;
            dy += 2;
        }
        if (err > 0) {
            x--;
            dx += 2;
            err += dx - (radius << 1);
        }
    }
}

void
CyclicMonoDrawer::drawCircleFill(int x0, int y0, int radius, color_t c)
{
    int x = radius-1;
    int y = 0;
    int dx = 1;
    int dy = 1;
    int err = dx - (radius << 1);

    while (x >= y) {
        drawHLine(x0 - x, x0 + x, y0 + y, c);
        drawHLine(x0 - x, x0 + x, y0 - y, c);
        drawVLine(x0 - y, y0 - x, y0 + x, c);
        drawVLine(x0 + y, y0 - x, y0 + x, c);

        if (err <= 0) {
            y++;
            err += dy;
            dy += 2;
        }
        if (err > 0) {
            x--;
            dx += 2;
            err += dx - (radius << 1);
        }
    }
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Method definitions
 *----------------------------------------------------------------------
 */

void
CyclicMonoDrawer::drawImage(int x, int y, MonoImage * image, bool blend, bool centered, bool offset)
{
    if (centered) {
        x -= (image->width() / 2);
        y -= (image->height() / 2);
    }
    if (offset) {
        x += image->drawOffsetX();
        y += image->drawOffsetY();
    }
    if (blend && image->hasAlpha()) {
        for (int y2 = 0; y2 < image->height(); y2++) {
            for (int x2 = 0; x2 < image->width(); x2++) {
                if (image->getDotAlpha(x2, y2) != 0) {
                    color_t c = (color_t)image->getDot(x2, y2);
                    drawDot(x + x2, y + y2, c);
                }
            }
        }
    } else {
      for (int y2 = 0; y2 < image->height(); y2++) {
          for (int x2 = 0; x2 < image->width(); x2++) {
              color_t c = (color_t)image->getDot(x2, y2);
              drawDot(x + x2, y + y2, c);
          }
      }
    }
}

void
CyclicMonoDrawer::drawImageCentered(int x, int y, MonoImage * image)
{
    drawImage(x, y, image, false, true, false);
}

void
CyclicMonoDrawer::drawImageBlend(int x, int y, MonoImage * image)
{
    drawImage(x, y, image, true, false, false);
}

void
CyclicMonoDrawer::drawImageBlendCentered(int x, int y, MonoImage * image)
{
    drawImage(x, y, image, true, true, false);
}

void
CyclicMonoDrawer::drawImageOffset(int x, int y, MonoImage * image)
{
    drawImage(x, y, image, false, false, true);
}

void
CyclicMonoDrawer::drawImageOffsetCentered(int x, int y, MonoImage * image)
{
    drawImage(x, y, image, false, true, true);
}

void
CyclicMonoDrawer::drawImageBlendOffset(int x, int y, MonoImage * image)
{
    drawImage(x, y, image, true, false, true);
}

void
CyclicMonoDrawer::drawImageBlendOffsetCentered(int x, int y, MonoImage * image)
{
    drawImage(x, y, image, true, true, true);
}
