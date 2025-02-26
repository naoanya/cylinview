/**********************************************************************/
/**
 * @brief  Screen Configuration
 * @author naoa
 */
/**********************************************************************/
#pragma once
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Include files
 *----------------------------------------------------------------------
 */

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Definitions
 *----------------------------------------------------------------------
 */

#define CV_HEIGHT           (128)                           // One display height in pixel
#define CV_WIDTH            (32)                            // One display width in pixel
#define CV_MARGIN           (39)                            // Margin between displays in pixel
#define CV_PIXELS           (CV_HEIGHT * CV_WIDTH)          // One display pixels
#define CV_ONE_FRAME_BYTES  (CV_PIXELS / 8)                 // One display frame data bytes
#define CV_DISPLAYS         (16)                            // Count of displays
#define CV_FRAME_BYTES      (CV_ONE_FRAME_BYTES * CV_DISPLAYS)  // Displays frame data bytes (Not include margin pixels)
#define CV_DISTANCE         (CV_WIDTH + CV_MARGIN)          // Distance between displays in pixel
#define CV_V_WIDTH          (CV_DISTANCE * CV_DISPLAYS)     // Display width includes margin
#define CV_V_PIXELS         (CV_V_WIDTH * CV_HEIGHT)        // Display pixels includes margin

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Definitions
 *----------------------------------------------------------------------
 */

typedef uint8_t color_t;

#define DISP_COLOR_WHITE     (0x01)
#define DISP_COLOR_BLACK     (0x00)
