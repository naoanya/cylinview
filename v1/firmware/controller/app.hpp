/**********************************************************************/
/**
 * @brief  Applications
 * @author naoa
 */
/**********************************************************************/
#pragma once
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Include files
 *----------------------------------------------------------------------
 */
#include <Arduino.h>
#include <cstdint>

#include "interval_timer.hpp"
#include "pseudo_rand.hpp"

#include "screen_config.hpp"
#include "mono_screen.hpp"
#include "cyclic_mono_screen.hpp"
#include "cyclic_mono_drawer.hpp"

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Definitions
 *----------------------------------------------------------------------
 */

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Class definitions
 *----------------------------------------------------------------------
 */

class App
{
public:
    explicit App();
    virtual ~App();

public:
    void init(void);
    void loop(
        uint32_t timeUs,
        float angle
    );
    void render(uint8_t * buffer);
    void setAutoModeChange(bool enable, int intervalMs);
    void setMode(int mode);
    void setAngle(float angle);

public:
    void render(void);
    void render_mode_0(void);
    void render_mode_1(void);
    void render_mode_2(void);
    void render_mode_3(void);
    void render_mode_4(void);
    void render_mode_5(void);
    void render_mode_6(void);
    void render_mode_7(void);

public:
    static uint32_t getRand(void);
    static int angle2xpos(float angle_);

public:
    CyclicMonoScreen screen_;
    CyclicMonoDrawer drawer_;
    
    int rendermode_;    
    int maxRenderMode_ = 5;

    bool autoRenderModeChange_ = true;
    IntervalTimer autoRenderModeChangeTimer_;

    float angle_;
};
