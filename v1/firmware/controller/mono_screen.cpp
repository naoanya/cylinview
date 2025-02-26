/**********************************************************************/
/**
 * @brief  Monochrome (8bit Packed) Screen
 * @author naoa
 */
/**********************************************************************/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Include files
 *----------------------------------------------------------------------
 */
#include <cstring>
#include <utility> // for std::swap
#include "mono_screen.hpp"

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Definitions - Debug
 *----------------------------------------------------------------------
 */

#define DP_PREFIX_STR   "MonoScreen: "

//#define DP__(...)   printf("DEBUG: " DP_PREFIX_STR __VA_ARGS__)
#define DP__(...)

#define DP_INFO__(...)   printf("INFO: " DP_PREFIX_STR __VA_ARGS__)
//#define DP_INFO__(...)

#define DP_ERROR__(...)   printf("ERROR: " DP_PREFIX_STR __VA_ARGS__)
//#define DP_ERROR__(...)

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Function definitions
 *----------------------------------------------------------------------
 */

MonoScreen::MonoScreen()
{
}

MonoScreen::~MonoScreen()
{
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Function definitions
 *----------------------------------------------------------------------
 */

void
MonoScreen::clear(color_t c)
{
    memset(buffer_, (c == DISP_COLOR_WHITE)? 0xFF : 0x00, CV_ONE_FRAME_BYTES);
}

void
MonoScreen::setDot(int x, int y, color_t c)
{
    #if 1
    std::swap(x, y);
    #endif

    #if 0
    int offset_stride = (y / 8) * CV_HEIGHT;
    int offset_bit    = y % 8;
    #else
    int offset_stride = (y >> 3) << 7;
    int offset_bit    = y & 7;
    #endif

    if (c == DISP_COLOR_WHITE)
        buffer_[x + offset_stride] |= (0x01 << offset_bit);
    else
        buffer_[x + offset_stride] &= ~(0x01 << offset_bit);
}

color_t
MonoScreen::getDot(int x, int y)
{
    #if 1
    std::swap(x, y);
    #endif
    
    #if 0
    int offset_stride = (y / 8) * CV_HEIGHT;
    int offset_bit    = y % 8;
    #else
    int offset_stride = (y >> 3) << 7;
    int offset_bit    = y & 7;
    #endif

    return (buffer_[x + offset_stride] >> offset_bit) & 0x01;
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Function definitions
 *----------------------------------------------------------------------
 */

void
MonoScreen::setBuffer(color_t * buffer)
{
    buffer_ = buffer;
}

color_t *
MonoScreen::getBuffer(void)
{
    return buffer_;
}
