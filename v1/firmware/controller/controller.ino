/**********************************************************************/
/**
 * @brief  CylinView Main Controller
 * @author naoa
 */
/**********************************************************************/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Include files
 *----------------------------------------------------------------------
 */

#include <stdlib.h>
#include <math.h>

#include "led.hpp"
#include "interval_timer.hpp"
#include "serialcmd.hpp"
#include "circular_buffer.hpp"

#include "motor.hpp"
#include "encoder.hpp"
#include "spi_i2c_bridge.hpp"
#include "screen_config.hpp"
#include "mono_screen.hpp"
#include "cyclic_mono_screen.hpp"
#include "cyclic_mono_drawer.hpp"
#include "app.hpp"

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

#ifndef MAX3
#define MAX3(x,y,z)     (MAX(MAX((x),(y)),(z)))
#endif

#ifndef MIN
#define MIN(x,y)        (((x) <= (y))? (x) : (y))
#endif

#ifndef MIN3
#define MIN3(x,y,z)     (MIN(MIN((x),(y)),(z)))
#endif

#ifndef SIGNUM
#define SIGNUM(x)       (((x) > 0)? (1) : ((x) < 0)? (-1) : (0))
#endif

#ifndef CONSTRAIN
#define CONSTRAIN(x,low,high)   ((x) < (low)? (low) : ((x) > (high)? (high) : (x)))
#endif

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Definitions - Debug
 *----------------------------------------------------------------------
 */

#define ENABLE_SETUP_SERIAL_HOST_WAIT   (0)
#define SETUP_SERIAL_HOST_WAIT_MS       (3000) // startup wait after serial begin for host pc connection

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Definitions
 *----------------------------------------------------------------------
 */

#define CIRCULAR_BUFFER_NUM             (2)
#define ENCODER_USE_SPI                 (1)

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Definitions
 *----------------------------------------------------------------------
 */

static const int pin_pio_spi_rx_     = 0;
static const int pin_pio_spi_cs_     = 1;
static const int pin_pio_spi_sck_    = 2;
static const int pin_pio_spi_tx_     = 3;

static const int pin_mot_in_1_       = 6;
static const int pin_mot_in_2_       = 7;

static const int pin_spi1_rx_        = 12;
static const int pin_spi1_cs_        = 13;
static const int pin_spi1_sck_       = 14;
static const int pin_spi1_tx_        = 15;
static const int pin_spi0_rx_        = 16;
static const int pin_spi0_cs_        = 17;
static const int pin_spi0_sck_       = 18;
static const int pin_spi0_tx_        = 19;

static const int pin_enc_pwm_        = 28;

static const int pin_buildin_led_   = PIN_LED; // 25

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Definitions
 *----------------------------------------------------------------------
 */

static float getAngle(void);
static uint16_t getRawAngle(void);
static void commandParser(SerialCmd & cmd);

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Definitions
 *----------------------------------------------------------------------
 */

static LED onBoardLed_(pin_buildin_led_, 500);
static bool core0SetupDone_ = false;
static bool core1SetupDone_ = false;
static SerialCmd cmd_(&Serial);
static IntervalTimer debugTimer_;

static uint8_t rawbuffer_[CV_FRAME_BYTES * CIRCULAR_BUFFER_NUM];
static CircularBuffer buffer_;
static SpiI2cBridge spi2i2cbridge_;

static App app_;
static float angle_ = 0;
static uint16_t angleOffset_ = 12900;

static IntervalTimer encMonTimer_;
static bool encoderMonitor_ = false;

static int fps_ = 0;
static bool fpsMonitor_ = false;

static bool enableSpiRender_ = true;

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Function definitions - Core 0
 *----------------------------------------------------------------------
 */

void setup()
{
  //stdio_init_all();

  //
  // Setup debug serial
  //

  Serial.begin(115200);
  #if ENABLE_SETUP_SERIAL_HOST_WAIT
  delay(SETUP_SERIAL_HOST_WAIT_MS);
  #endif
  Serial.printf("START SETUP core 0\n");

  //
  // Setup modules
  //

  debugTimer_.setIntervalMs(1000);
  encMonTimer_.setIntervalMs(100);
  
  // Init Buffers
  buffer_.setBuffer(rawbuffer_, sizeof(rawbuffer_), CIRCULAR_BUFFER_NUM);

  // Init SPI for SPI2I2C Bridge
  spi2i2cbridge_.init(
    pin_spi0_rx_,
    pin_spi0_cs_,
    pin_spi0_sck_,
    pin_spi0_tx_,
    pin_spi1_rx_,
    pin_spi1_cs_,
    pin_spi1_sck_,
    pin_spi1_tx_
  );

  // Init Encoder
  encoder_set_angle_offset(angleOffset_);
  #if ENCODER_USE_SPI
  encoder_init_spi(
    pin_pio_spi_rx_,
    pin_pio_spi_cs_,
    pin_pio_spi_sck_,
    pin_pio_spi_tx_
  );
  #else
  encoder_init_pwm(pin_enc_pwm_);
  #endif

  // Init Motor
  motor_init(pin_mot_in_1_, pin_mot_in_2_);

  // Init App
  app_.init();

  // wait i2c-spi-bridge
  while (!spi2i2cbridge_.sendPing(0)) {;}
  while (!spi2i2cbridge_.sendPing(1)) {;}

  // setup i2c-spi-bridge
  spi2i2cbridge_.sendSetIDDirection(0, true);
  spi2i2cbridge_.sendSetIDDirection(1, false);

  //
  // Setup done
  //

  core0SetupDone_ = true;
  Serial.printf("start loop core 0\n");
}

void loop()
{
  //
  // Serial Commands
  //

  if (cmd_.loop() != 0) {
    commandParser(cmd_);
  }

  //
  // Main processes
  //

  angle_ = getAngle();
  
  app_.loop(micros(), angle_);

  if (buffer_.getWriteReady()) {
    // Current buffer is now writable.

    // Render    
    app_.render(buffer_.getWriteBufferPtr());

    // Set next write buffer
    buffer_.nextWriteBuffer();
    
    fps_++;
  }

  //
  // Debug processes
  //

  onBoardLed_.loop();

  if (debugTimer_.check()) {
    //Serial.printf("Core Temp = %2.1f C\n", analogReadTemp());
    if (fpsMonitor_) {
      Serial.printf("%d\n", fps_);
    }
    fps_ = 0;
  }

  if (encMonTimer_.check()) {
    if (encoderMonitor_) {
      Serial.printf("%f\n", angle_);
    }
  }
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Function definitions - Core 1
 *----------------------------------------------------------------------
 */

void setup1(void)
{
  //
  // Wait core0 setup
  //

  while (1) {
    if (core0SetupDone_) break;
  }

  //
  // Setup done
  //

  core1SetupDone_ = true;
  Serial.printf("start loop core 1\n");
}

void loop1(void)
{
  //
  // Main processes
  //

  if (enableSpiRender_ && buffer_.getReadReady()) {
    // transfer data available, start spi transfers

    // Send frame data to spi-i2c-bridge
    spi2i2cbridge_.sendFrameDataParallel(buffer_.getReadBufferPtr(), CV_FRAME_BYTES);
    
    // transfer completed, set next read buffers
    buffer_.nextReadBuffer();
  }
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Function definitions
 *----------------------------------------------------------------------
 */

static float getAngle(void)
{
  #if ENCODER_USE_SPI
  return encoder_get_angle_spi();
  #else
  return encoder_get_angle_pwm();
  #endif
}

static uint16_t getRawAngle(void)
{
  #if ENCODER_USE_SPI
  return encoder_get_raw_data_spi();
  #else
  return encoder_get_raw_data_pwm();
  #endif
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Function definitions - Debug Command Console
 *----------------------------------------------------------------------
 */

#define CMDSTART                if (0) {;}
#define ISCMD(name)             else if (strcmp(cmd.getCmd(), name) == 0)
#define ISCMD2(cmdname)         else if (strcmp(cmd.getParam(0), cmdname) == 0)
#define GETPARAM(index, type)   SerialCmd::stringTo ## type(cmd.getParam(index))

static void commandParser(SerialCmd & cmd)
{
  Serial.printf("CMD : %s\n", cmd.getCmd());

  CMDSTART
  ISCMD("MODE")
  {
    int mode = GETPARAM(0, Int);
    app_.setMode(mode);
    Serial.printf("mode = %d\n", mode);
  }
  ISCMD("FPSMON")
  {
    fpsMonitor_ = GETPARAM(0, Int);
  }
  ISCMD("M")
  {
    int power = GETPARAM(0, Int);
    motor_set_power(power);
  }
  ISCMD("MB")
  {
    int brake = GETPARAM(0, Int);
    motor_set_brake(brake);
  }
  ISCMD("MS")
  {
    int decay = GETPARAM(0, Int);
    motor_set_decay_mode(decay);
  }
  ISCMD("MOTPIN")
  {
    int number = GETPARAM(0, Int);
    int logic  = GETPARAM(1, Int);
    if (number == 0) {
      digitalWrite( pin_mot_in_1_, (logic == 0)? LOW : HIGH );
    } else {
      digitalWrite( pin_mot_in_2_, (logic == 0)? LOW : HIGH );
    }
  }
  ISCMD("E")
  {
    Serial.printf("angle = %f\n", getAngle());
  }
  ISCMD("ER")
  {
    Serial.printf("rawangle = %d\n", getRawAngle());
  }
  ISCMD("EO")
  {
    bool offset = GETPARAM(0, Int);
    angleOffset_ = offset;
    encoder_set_angle_offset(angleOffset_);
  }
  ISCMD("EOG")
  {
    Serial.printf("angleOffset_ = %d\n", angleOffset_);
  }
  ISCMD("EMON")
  {
    encoderMonitor_ = GETPARAM(0, Int);
  }
  ISCMD("ENA_SPI_XFER")
  {
    enableSpiRender_ = GETPARAM(0, Int);
  }
  ISCMD("MOVETO")
  {
    float target = GETPARAM(0, Float);
    Serial.printf("target = %f\n", target);
    motor_set_power((target >= 0)? 70 : -70);
    float pre = getAngle();
    float moved = 0;
    uint32_t limit = 0xFFFFFFFF;
    while(1) {
      float cur = getAngle();
      //Serial.printf("%f - %f : %f\n", pre, cur, moved);
      if (cur > pre && (cur - pre > M_PI)) cur -= (2 * M_PI);
      else if (pre - cur > M_PI) cur += (2 * M_PI);
      moved += (cur - pre);
      if (target > 0 && (moved >= target)) break;
      else if (moved <= target) break;
      pre = cur;
      limit--;
      if (limit == 0) break;
    }
    motor_set_brake(true);
    Serial.printf("done\n");
  }
  ISCMD("SPI_TEST1")
  {
    uint8_t id = GETPARAM(0, Int);
    uint8_t tx = GETPARAM(1, Int);
    Serial.printf("tx = 0x%02x\n", tx);
    uint8_t rx = 0;
    spi2i2cbridge_.transfer(id, &tx, &rx, 1);
    Serial.printf("rx = 0x%02x\n", rx);
  }
  ISCMD("SPI_TEST2")
  {
    uint8_t id = GETPARAM(0, Int);
    for (int i = 0; i < 3; i++) {
      { uint8_t buffer[11]; memset((void*)buffer, 0, sizeof(buffer)); spi2i2cbridge_.transfer(id, buffer, NULL, sizeof(buffer)); }
      { uint8_t buffer[3]; memset((void*)buffer, 0, sizeof(buffer)); spi2i2cbridge_.transfer(id, buffer, NULL, sizeof(buffer)); }
      { uint8_t buffer[1]; memset((void*)buffer, 0, sizeof(buffer)); spi2i2cbridge_.transfer(id, buffer, NULL, sizeof(buffer)); }
    }
  }
  ISCMD("SPI_TEST3")
  {
    uint8_t id = GETPARAM(0, Int);
    memset((void*)rawbuffer_, 0, sizeof(rawbuffer_));
    spi2i2cbridge_.transfer(id, rawbuffer_, NULL, sizeof(rawbuffer_));
  }
  ISCMD("SPI_PING")
  {
    uint8_t id = GETPARAM(0, Int);
    Serial.printf("start ping %d\n", id);
    bool result = spi2i2cbridge_.sendPing(id);
    Serial.printf("finish ping %d\n", result);
  }
  ISCMD("SPI_DIR")
  {
    uint8_t id = GETPARAM(0, Int);
    uint8_t on = GETPARAM(1, Int);
    spi2i2cbridge_.sendSetIDDirection(id, on);
  }
  ISCMD("SPI_LED")
  {
    uint8_t id = GETPARAM(0, Int);
    uint8_t on = GETPARAM(1, Int);
    spi2i2cbridge_.sendSetLED(id, on);
  }
  ISCMD("SPI_RESET")
  {
    uint8_t id = GETPARAM(0, Int);
    spi2i2cbridge_.sendHardReset(id);
  }
  ISCMD("RESET")
  {
    watchdog_enable(1, 1);
    while(1);
  }
  ISCMD("EXAMPLE")
  {
    CMDSTART
    ISCMD2("EXAMPLE1") { Serial.printf("CMD : Sub Example Command 1\n"); }
    ISCMD2("EXAMPLE2") { Serial.printf("CMD : Sub Example Command 2\n"); }
  }
}
