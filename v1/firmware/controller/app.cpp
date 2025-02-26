/**********************************************************************/
/**
 * @brief  Applications
 * @author naoa
 */
/**********************************************************************/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Include files
 *----------------------------------------------------------------------
 */
#include <cstdint>
#include <cmath>
#include <functional>

#include "motor.hpp"

#include "image_data.hpp"
#include "snow.hpp"
//#include "character.hpp"

#include "app.hpp"

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Definitions
 *----------------------------------------------------------------------
 */

static PseudoRand rnd_;

#define GRAINS      (100)
static Snow snow;
static Snow::Grain grains[GRAINS];

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Function definitions
 *----------------------------------------------------------------------
 */

App::App()
{
    rendermode_ = 0;

    rnd_.setSeed(rand(), rand(), rand(), rand());

    snow.setRandCallback(getRand);
    snow.setSnowGrains(grains, GRAINS);
}

App::~App()
{

}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Function definitions
 *----------------------------------------------------------------------
 */

void
App::init(void)
{
    setAutoModeChange(false, 10 * 1000);
}

void
App::loop(
    uint32_t timeUs,
    float angle
) {
    angle_ = angle;

    if (autoRenderModeChangeTimer_.check()) {
        if (autoRenderModeChange_) {
            rendermode_++;
            if (rendermode_ > maxRenderMode_) rendermode_ = 0;
        }
    }
}

void
App::render(uint8_t * buffer)
{
    // Setup render buffer
    for (int i = 0; i < CV_DISPLAYS; i++) {
      MonoScreen * monoscreen = screen_.getMonoScreen(i);
      monoscreen->setBuffer( buffer + (i * CV_ONE_FRAME_BYTES) );
    }

    // Setup Drawer
    drawer_.init(&screen_);

    render();
}

void
App::setAutoModeChange(bool enable, int intervalMs)
{
    autoRenderModeChange_ = enable;
    autoRenderModeChangeTimer_.setIntervalMs(intervalMs);
}

void
App::setMode(int mode)
{
    rendermode_ = mode;
}

void
App::setAngle(float angle)
{
    angle_ = angle;
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Function definitions
 *----------------------------------------------------------------------
 */

void
App::render(void)
{
    switch (rendermode_)
    {
    case 0: render_mode_0(); break;
    case 1: render_mode_1(); break;
    case 2: render_mode_2(); break;
    case 3: render_mode_3(); break;
    case 4: render_mode_4(); break;
    case 5: render_mode_5(); break;
    case 6: render_mode_6(); break;
    case 7: render_mode_7(); break;
    default: break;
    }
}

void
App::render_mode_0(void)
{
#if 0
    int xpos = angle2xpos(angle_);
    for (int i = 0; i < 8; i++) {
        const int r = 50;
        int x = (((r * 2) + 3) * i) - xpos;
        int y = 128 / 2;
        drawer_.drawCircle(x, y, r - 40, 1);
        drawer_.drawCircle(x, y, r - 30, 1);
        drawer_.drawCircle(x, y, r - 20, 1);
        drawer_.drawCircle(x, y, r - 10, 1);
        drawer_.drawCircle(x, y, r     , 1);
    }
#endif
    //drawer_.clearFrame();

    static const mono_images_t * frames = &image_badapple_frames;
    static int frameno = 0;

    int xpos = angle2xpos(angle_);
    MonoImage image(&frames->images_[frameno]);
    drawer_.drawImageOffset(-xpos, CV_HEIGHT / 2, &image);

    frameno++;
    if (frameno >= frames->count_) frameno = 0;
}

#if 0
void
App::render_mode_1(void)
{
    drawer_.clearFrame();

    auto render_mode_1_draw = [&](int cx, int cy, float * deg)
    {
        const float div = 1.0;
        const int   limit = (1 * CV_V_WIDTH);
        
        int x;
        float y;
        int nodiff;
        int counter;
        float ydiff = tan((2 * M_PI) * (*deg / 360.0));

        x = cx;
        y = cy;
        nodiff = 0;
        counter = 0;
        
        while (1) {
            drawer_.drawDot(x + 0, (int)y + 0, 1);
            drawer_.drawDot(x + 0, (int)y + 1, 1);
            drawer_.drawDot(x + 0, (int)y - 1, 1);
            drawer_.drawDot(x + 1, (int)y + 0, 1);
            drawer_.drawDot(x - 1, (int)y + 0, 1);
            x = x + 1;
            y = y + ydiff;
            int iy = (int)y;
            if (iy >= 128 || iy < 0) break;
            if (iy == cy) nodiff++;
            if (nodiff >= (CV_V_WIDTH / 2)) break;
            counter++;
            if (counter > limit) break;
        }

        x = cx;
        y = cy;
        nodiff = 0;
        counter = 0;

        while (1) {
            drawer_.drawDot(x, (int)y + 1, 1);
            drawer_.drawDot(x, (int)y + 0, 1);
            drawer_.drawDot(x, (int)y - 1, 1);
            x = x - 1;
            y = y - ydiff;
            int iy = (int)y;
            if (iy >= 128 || iy < 0) break;
            if (iy == cy) nodiff++;
            if (nodiff >= (CV_V_WIDTH / 2)) break;
            counter++;
            if (counter > limit) break;
        }
        
        *deg += div;
        if (*deg >= 360.0) *deg = *deg - 360.0;
    };

    static float degree0 = 45.0 * 0;
    static float degree1 = 45.0 * 1;
    static float degree2 = 45.0 * 2;
    static float degree3 = 45.0 * 3;
    render_mode_1_draw((32 / 2), 128 / 2, &degree0);
    render_mode_1_draw((32 / 2), 128 / 2, &degree1);
    render_mode_1_draw((32 / 2), 128 / 2, &degree2);
    render_mode_1_draw((32 / 2), 128 / 2, &degree3);
}

void
App::render_mode_2(void)
{
    drawer_.clearFrame();

    int xpos = angle2xpos(angle_);
    for (int i = 0; i < 8; i++) {
        const int r = 50;
        int x = (((r * 2) + 3) * i) - xpos;
        int y = 128 / 2;
        drawer_.drawCircle(x, y, r - 40, 1);
        drawer_.drawCircle(x, y, r - 30, 1);
        drawer_.drawCircle(x, y, r - 20, 1);
        drawer_.drawCircle(x, y, r - 10, 1);
        drawer_.drawCircle(x, y, r     , 1);
    }
}

void
App::render_mode_3(void)
{
    drawer_.clearFrame();

    int xpos = angle2xpos(angle_);
    snow.loop();
    for (int i = 0; i < GRAINS; i++) {
        Snow::Grain * g = &grains[i];
        drawer_.drawDot(g->x_ - xpos, g->y_);
    }
}

void
App::render_mode_4(void)
{
    drawer_.clearFrame();

    static const mono_images_t * frames = &image_anim_test_frames;
    static int frameno = 0;

    int xpos = angle2xpos(angle_);
    MonoImage image(&frames->images_[frameno]);
    drawer_.drawImageOffset(-xpos, 64, &image);

    frameno++;
    if (frameno >= frames->count_) frameno = 0;
}

void
App::render_mode_5(void)
{
    drawer_.clearFrame();

    int xpos = angle2xpos(angle_);
    snow.loop();
    for (int i = 0; i < GRAINS; i++) {
        Snow::Grain * g = &grains[i];
        drawer_.drawDot(g->x_ - xpos, g->y_);
    }

    //static const mono_images_t * frames = &image_anim_test_frames;
    static const mono_images_t * frames = &image_sky1_frames;
    //static const mono_images_t * frames = &image_sky2_frames;
    static int frameno = 0;
    static int framexpos = 0;

    MonoImage image(&frames->images_[frameno]);
    drawer_.drawImageBlendOffset(framexpos - xpos, 64, &image);

    framexpos++;
    if (framexpos >= CV_V_WIDTH) framexpos = 0;

    frameno++;
    if (frameno >= frames->count_) frameno = 0;
}

void
App::render_mode_6(void)
{
    const int fps = 70;
    const int motspeed = 80;

    static int m6State = 0;
    static int m6Counter0 = 0;
    static int m6Counter1 = 0;
    static int m6Counter2 = 0;
    static int m6Counter3 = 0;
    static bool m6Inited = false;

    static auto m6Init = [&](const std::function<void(void)> & func) {
        if (!m6Inited) {
            m6Counter0 = 0;
            m6Counter1 = 0;
            m6Counter2 = 0;
            m6Counter3 = 0;
            m6Inited = true;
            func();
        }
    };

    static auto m6Timer = [&](const std::function<void(void)> & func, int timeout) {
        m6Counter0++;
        if (m6Counter0 >= timeout) {
            m6Counter0 = 0;
            func();
        }
    };

    static auto m6GotoNextState = [&](int nextstate = -1) {
        if (nextstate >= 0) {
            m6State = nextstate;
        } else {
            m6State++;
        }
        m6Inited = false;
    };

    int xpos = angle2xpos(angle_);
    switch (m6State)
    {
    case 0: // Show display numbers
    {
        m6Init([&](){
            motor_set_brake(true);
        });

        drawer_.clearFrame();
    
        for (int i = 0; i <= m6Counter1; i++) {
            MonoImage image(&image_dispnum_frames.images_[i]);
            drawer_.drawImageBlendCentered((i * CV_DISTANCE)  + CV_WIDTH / 2, CV_HEIGHT / 2, &image);
        }

        m6Timer([&](){
            m6Counter1++;
            if (m6Counter1 >= CV_DISPLAYS) {
                m6GotoNextState();
            }
        }, fps / 8);
    } break;
    case 1: // Start rotating while zero position
    {
        m6Init([&](){
            motor_set_power(-motspeed);
        });

        if (xpos <= 10) {
            m6GotoNextState();
        }
    } break;
    case 2: // Interval
    {
        m6Init([&](){
            m6Counter1 = CV_DISPLAYS - 1;
        });

        drawer_.clearFrame();

        for (int i = 0; i <= m6Counter1; i++) {
            MonoImage image(&image_dispnum_frames.images_[i]);
            drawer_.drawImageBlendCentered((i * CV_DISTANCE) + CV_WIDTH / 2 - xpos, CV_HEIGHT / 2, &image);
        }

        m6Timer([&](){
            m6GotoNextState();
        }, fps * 5);
    } break;
    case 3: // Show center line
    {
        m6Init([&](){});

        drawer_.clearFrame();
        
        drawer_.drawHLine(CV_WIDTH / 2 - xpos, CV_WIDTH / 2 + m6Counter1 - xpos, CV_HEIGHT / 2);
        drawer_.drawHLine(CV_WIDTH / 2 - xpos, CV_WIDTH / 2 - m6Counter1 - xpos, CV_HEIGHT / 2);

        m6Counter1 += ((int)(fps / 10.0));
        if (m6Counter1 >= (CV_V_WIDTH / 2)) {
            m6GotoNextState();
        }
    } break;
    case 4: // Show white screen gradually
    {
        m6Init([&](){
            m6Counter1 = 1;
        });

        drawer_.clearFrame();

        for (int i = 0; i < m6Counter1; i++) {
            drawer_.drawHLine(0 - xpos, CV_V_WIDTH - 1 - xpos, CV_HEIGHT / 2 + i);
            drawer_.drawHLine(0 - xpos, CV_V_WIDTH - 1 - xpos, CV_HEIGHT / 2 - i);
        }

        m6Counter1 += ((int)(fps / 20.0));
        if (m6Counter1 >= (CV_HEIGHT / 2)) {
            m6GotoNextState();
        }
    } break;
    case 5: // Interval
    {
        m6Init([&](){});

        drawer_.clearFrame(DISP_COLOR_WHITE);

        m6Timer([&](){
            m6GotoNextState();
        }, fps);
    } break;
    case 6: // Show CylinView logo
    {
        m6Init([&](){});

        drawer_.clearFrame(DISP_COLOR_WHITE);

        MonoImage image(&image_title_cylinview_frames.images_[0]);
        drawer_.drawImageBlendCentered(CV_WIDTH / 2 - xpos, CV_HEIGHT / 2, &image);

        m6Timer([&](){
            m6GotoNextState();
        }, fps * 5);
    } break;
    case 7: // Hide logo gradually, and stop if zero position
    {
        m6Init([&](){});

        drawer_.clearFrame();

        for (int i = 0; i < ((CV_HEIGHT / 2) - m6Counter1); i++) {
            drawer_.drawHLine(0, CV_V_WIDTH - 1, CV_HEIGHT / 2 + i);
            drawer_.drawHLine(0, CV_V_WIDTH - 1, CV_HEIGHT / 2 - i);
        }

        // not enough cpu power
        //MonoImage image(&image_title_cylinview_frames.images_[0]);
        //drawer_.drawImageBlendCentered(CV_WIDTH / 2 - xpos, CV_HEIGHT / 2, &image);

        m6Counter1 += ((int)(fps / 20.0));
        if (m6Counter1 >= (CV_HEIGHT / 2)) {
            m6GotoNextState();
        }
    } break;
    case 8: // Wait zero position and stop
    {
        m6Init([&](){});

        drawer_.clearFrame();

        if (xpos <= 10) {
            motor_set_brake(true);
            m6GotoNextState();
        }
    } break;
    case 9: // Interval
    {
        m6Init([&](){
            motor_set_brake(true);
        });
        m6Timer([&](){
            m6GotoNextState();
        }, fps * 1);
    } break;
    case 10: // Show snow
    {
        m6Init([&](){});
        
        drawer_.clearFrame();

        snow.loop();
        for (int i = 0; i < GRAINS; i++) {
            Snow::Grain * g = &grains[i];
            drawer_.drawDot(g->x_ - xpos, g->y_);
        }
    
        m6Timer([&](){
            m6GotoNextState();
        }, fps * 2.5);
    } break;
    case 11: // Show snow, and show ground gradually
    {
        m6Init([&](){});
        
        drawer_.clearFrame();

        snow.loop();
        for (int i = 0; i < GRAINS; i++) {
            Snow::Grain * g = &grains[i];
            drawer_.drawDot(g->x_ - xpos, g->y_);
        }
        
        drawer_.drawRectFill(
            0             , CV_HEIGHT - m6Counter1,
            CV_V_WIDTH - 1, CV_HEIGHT - 1
        );
    
        m6Timer([&](){
            m6Counter1++;
            if (m6Counter1 >= 8) {
                m6GotoNextState();
            }
        }, fps * 0.6);
    } break;
    case 12: // Character entry
    {
        m6Init([&](){});

        drawer_.clearFrame();

        snow.loop();
        for (int i = 0; i < GRAINS; i++) {
            Snow::Grain * g = &grains[i];
            drawer_.drawDot(g->x_ - xpos, g->y_);
        }
        
        drawer_.drawRectFill(
            0             , CV_HEIGHT - 8,
            CV_V_WIDTH - 1, CV_HEIGHT - 1
        );

        static const mono_images_t * animations[] = {
            &image_entry_frames
        };
        const mono_images_t * frames = animations[m6Counter2];
        MonoImage image(&frames->images_[m6Counter1]);
        drawer_.drawImageBlendOffset(CV_WIDTH / 2, CV_HEIGHT / 2, &image);
    
        m6Counter3++;
        if (m6Counter3 > 2) {
          m6Counter3 = 0;
          m6Counter1++;
          if (m6Counter1 >= frames->count_) {
              m6Counter1 = 0;
              m6Counter2++;
              if (m6Counter2 >= (sizeof(animations) / sizeof(mono_images_t*))) {
                  m6GotoNextState();
              }
          }
        }
    } break;
    case 13: // Interval
    {
        m6Init([&](){});

        drawer_.clearFrame();

        snow.loop();
        for (int i = 0; i < GRAINS; i++) {
            Snow::Grain * g = &grains[i];
            drawer_.drawDot(g->x_ - xpos, g->y_);
        }

        drawer_.drawRectFill(
            0             , CV_HEIGHT - 8,
            CV_V_WIDTH - 1, CV_HEIGHT - 1
        );
    
        const mono_images_t * frames = &image_idle2_frames;
        MonoImage image(&frames->images_[m6Counter1]);
        drawer_.drawImageBlendOffset(CV_WIDTH / 2, CV_HEIGHT / 2, &image);
    
        m6Counter3++;
        if (m6Counter3 > 2) {
          m6Counter3 = 0;
          m6Counter1++;
          if (m6Counter1 >= frames->count_) {
              m6Counter1 = 0;
              m6Counter2++;
              if (m6Counter2 >= 1) {
                  m6GotoNextState();
              }
          }
        }
    } break;
    case 14:
    {
        #if 0
        m6GotoNextState();
        #else
        motor_set_power(-motspeed);
        rendermode_ = 7;
        #endif
    } break;
    default: 
    {
        m6State = 0; 
    } break;
    }
}

void
App::render_mode_7(void)
{
    static Character character_;
    
    static bool inited_ = false;
    if (!inited_) {
        inited_ = true;
        character_.init();
        character_.setRandCallback(getRand);
        character_.setAuto();
    }

    int xpos = angle2xpos(angle_);

    drawer_.clearFrame();

    snow.loop();
    for (int i = 0; i < GRAINS; i++) {
        Snow::Grain * g = &grains[i];
        drawer_.drawDot(g->x_ - xpos, g->y_);
    }

    drawer_.drawRectFill(
        0             , CV_HEIGHT - 8,
        CV_V_WIDTH - 1, CV_HEIGHT - 1
    );

    MonoImage image(character_.getImage());
    drawer_.drawImageBlendOffset(CV_WIDTH / 2 - xpos + character_.getXpos(), CV_HEIGHT / 2, &image);
}
#endif

void App::render_mode_1(void) {};
void App::render_mode_2(void) {};
void App::render_mode_3(void) {};
void App::render_mode_4(void) {};
void App::render_mode_5(void) {};
void App::render_mode_6(void) {};
void App::render_mode_7(void) {};

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Function definitions - Utils
 *----------------------------------------------------------------------
 */

uint32_t
App::getRand(void)
{
    return rnd_.rand();
}

int
App::angle2xpos(float angle)
{
    return (angle / (2 * M_PI)) * CV_V_WIDTH;
}
