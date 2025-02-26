/**********************************************************************/
/**
 * @brief  SPI to I2C Bridge (SSD1306 / PIO Multi Channel)
 * @author naoa
 */
/**********************************************************************/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Include files
 *----------------------------------------------------------------------
 */
#include <utility> // for std::swap

#include <SPI.h>
#include <SPISlave.h>

#include "led.hpp"
#include "interval_timer.hpp"
#include "circular_buffer.hpp"
#include "ssd1306_multi_pio.hpp"

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

#define DISPLAY_WIDTH           (32)
#define DISPLAY_HEIGHT          (128)
#define DISPLAY_PIXELS          (DISPLAY_WIDTH * DISPLAY_HEIGHT)
#define DISPLAY_BYTES_PER_PIXEL ((float)1 / 8)
#define DISPLAY_BYTES           ((int)(DISPLAY_PIXELS * DISPLAY_BYTES_PER_PIXEL))

#define CIRCULAR_BUFFER_NUM     (2)
#define I2C_CHANNELS            (8)
#define BUFFER_SIZE             (DISPLAY_BYTES * I2C_CHANNELS)

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Definitions
 *----------------------------------------------------------------------
 */

static const int pin_i2c_base_      = 0; // 0 ~ 15, (SDA + SCL) * 8 ch = 16 pin

static const int pin_spi_rx_        = 16;
static const int pin_spi_cs_        = 17;
static const int pin_spi_sck_       = 18;
static const int pin_spi_tx_        = 19;

static const int pin_buildin_led_   = PIN_LED; // 25

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Definitions
 *----------------------------------------------------------------------
 */

static void calc_crc16_lookup_table(void);
static inline void calc_crc16(const uint8_t & data);
static void spiReceivedIrqCallback(uint8_t *data, size_t len);
static void spiSentIrqCallback();

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Definitions
 *----------------------------------------------------------------------
 */

static LED onBoardLed_(pin_buildin_led_, 500);
static bool core0SetupDone_ = false;
static bool core1SetupDone_ = false;
static IntervalTimer debugTimer_;

static SSD1306MultiPIO ssd1306mpio_;

static SPISettings spisettings(50 * 1000 * 1000, MSBFIRST, SPI_MODE0);

static const int SPI_STATE_SYNC1 = 0;
static const int SPI_STATE_SYNC2 = 1;
static const int SPI_STATE_CMD   = 2;
static const int SPI_STATE_OPT1  = 3;
static const int SPI_STATE_OPT2  = 4;
static const int SPI_STATE_OPT3  = 5;
static const int SPI_STATE_OPT4  = 6;
static const int SPI_STATE_DATA  = 7;
static const int SPI_STATE_CRC1  = 8;
static const int SPI_STATE_CRC2  = 9;

static const int SPI_CMD_NONE        = 0x00;
static const int SPI_CMD_GET_STATUS  = 0x01;
static const int SPI_CMD_START_FRAME = 0x02;
static const int SPI_CMD_SET_DATA    = 0x03;
static const int SPI_CMD_PING        = 0x04;
static const int SPI_CMD_OB_LED_ON   = 0x05;
static const int SPI_CMD_OB_LED_OFF  = 0x06;
static const int SPI_CMD_SET_ID_DIR0 = 0x07;
static const int SPI_CMD_SET_ID_DIR1 = 0x08;
static const int SPI_CMD_HARD_RESET  = 0xFE;

static const int SPI_SYNC1 = 0xAA;
static const int SPI_SYNC2 = 0x55;
static const int SPI_TXDATA_VALID_FLAG = 0x80;
static const int SPI_RSP_DATA_BUSY  = (1 << 1);
static const int SPI_RSP_DATA_PING  = (1 << 2);
static const int SPI_RSP_DATA_ERROR = (1 << 3);

static const int SPI_RSP_NONE        = 0x00;
static const int SPI_RSP_GET_STATUS  = 0x01;
static const int SPI_RSP_PING        = 0x02;
static const int SPI_RSP_ERROR       = 0x03;

static int        spiState_;
static uint8_t    spiCmd_;
static uint8_t    spiOpt1_;
static uint8_t    spiOpt2_;
static uint8_t    spiOpt3_;
static uint8_t    spiOpt4_;
static uint       spiDataBlockSize_;
static uint8_t    spiCrc1_;
static uint8_t    spiCrc2_;
static uint16_t   spiCrc_;

static uint8_t    spiTxBuffer_;
static int        spiResponseFlag_;

static uint8_t        rawbuffer_[BUFFER_SIZE * CIRCULAR_BUFFER_NUM];
static CircularBuffer buffer_;
static uint8_t *      wrBufferPtr_;
static uint           wrBufferAvail_;

static uint32_t   xfer_count_ = 0;
static bool       ob_led_on_ = true;

static uint32_t   fps_ = 0;

static uint16_t   crc16_lu_table[256]; 
static const uint16_t crc16_polynomial = 0x1021;

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

  // @note Debug: Used for GPIO toggle to measure processing time of spi transmit interrupt handler.
  //pinMode(22, OUTPUT); 

  // Init Buffers
  buffer_.setBuffer(rawbuffer_, sizeof(rawbuffer_), CIRCULAR_BUFFER_NUM);
  wrBufferPtr_   = buffer_.getWriteBufferPtr();
  wrBufferAvail_ = BUFFER_SIZE;

  // Init Display (and PIO I2C)
  ssd1306mpio_.init();

  // Init SPI
  calc_crc16_lookup_table();
  spiState_ = SPI_STATE_SYNC1;
  spiCmd_ = SPI_CMD_NONE;
  spiResponseFlag_ = SPI_RSP_NONE;

  SPISlave.setRX(pin_spi_rx_);
  SPISlave.setCS(pin_spi_cs_);
  SPISlave.setSCK(pin_spi_sck_);
  SPISlave.setTX(pin_spi_tx_);
  SPISlave.onDataRecv(spiReceivedIrqCallback);
  SPISlave.onDataSent(spiSentIrqCallback);
  SPISlave.begin(spisettings);

  //
  // Setup done
  //

  core0SetupDone_ = true;
  Serial.printf("start loop core 0\n");
}

void loop()
{
  //
  // Main processes
  //

  // @note core0 handling spi interrupts.

  //
  // Debug processes
  //

  if (ob_led_on_) {
    onBoardLed_.loop();
  }

#if 0
  if (debugTimer_.check()) {
    //Serial.printf("Core Temp = %2.1f C\n", analogReadTemp());
    //Serial.printf("xfer_count_ = %d\n", xfer_count_);
  }
#endif

#if 0
  // Test
  static bool buffersetuponce = true;
  if (buffersetuponce) {
    buffersetuponce = false;

    auto setDot = [](uint8_t * buf, int x, int y) {
      #if 1
      std::swap(x, y);
      #endif
      int offset_stride = (y / 8) * DISPLAY_WIDTH;
      int offset_bit    = y % 8;
      buf[x + offset_stride] |= (0x01 << offset_bit);
    };

    for (int id = 0; id < I2C_CHANNELS; id++) {
      uint8_t * buf = buffer_.getWriteBufferPtr() + (id * DISPLAY_BYTES);
      memset(buf, 0, DISPLAY_BYTES);
      setDot(buf,                 0,                  0);
      setDot(buf,                 0, DISPLAY_HEIGHT - 1);
      setDot(buf, DISPLAY_WIDTH - 1,                  0);
      setDot(buf, DISPLAY_WIDTH - 1, DISPLAY_HEIGHT - 1);
      for (int i = 0; i < DISPLAY_HEIGHT; i++) {
        setDot(buf, DISPLAY_WIDTH / 2, i);
      }
      for (int i = 0; i < DISPLAY_WIDTH; i++) {
        setDot(buf, i, DISPLAY_HEIGHT / 2);
      }
      for (int i = 0; i < DISPLAY_WIDTH; i++) {
        setDot(buf, i, i);
      }
    }
  }

  #if 1
  // Parallel
  ssd1306mpio_.writeFrameMulti(buffer_.getWriteBufferPtr());
  #else
  // Sequential
  for (int id = 0; id < I2C_CHANNELS; id++) {
    ssd1306mpio_.writeFrame(id, buffer_.getWriteBufferPtr(), DISPLAY_BYTES );
  }
  #endif

#endif
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

  if (buffer_.getReadReady()) {
    //Serial.printf("ReadBuffer Ready\n");
    ssd1306mpio_.writeFrameMulti(buffer_.getReadBufferPtr());

    // Set buffer status.
    {
      uint32_t irqstatus = save_and_disable_interrupts();
      buffer_.nextReadBuffer();
      restore_interrupts(irqstatus);
    }
  }

  //
  // Debug processes
  //

#if 0
  fps_++;
  if (debugTimer_.check()) {
    Serial.printf("fps_ = %d\n", fps_);
    fps_ = 0;
  }
#endif

}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Function definitions - Interrupt Callbacks
 *----------------------------------------------------------------------
 */

static void calc_crc16_lookup_table(void)
{
  for (uint16_t i = 0; i < 256; i++) {
    uint16_t crc = i << 8;
    for (int j = 0; j < 8; j++) {
      if (crc & 0x8000) crc = (crc << 1) ^ crc16_polynomial;
      else              crc <<= 1;
    }
    crc16_lu_table[i] = crc;
  }
}

static inline void calc_crc16(const uint8_t & data)
{
  spiCrc_ = (spiCrc_ << 8) ^ crc16_lu_table[((spiCrc_ >> 8) ^ data) & 0xFF];
}

//void spiReceivedIrqCallback(uint8_t *data, size_t len)
void __time_critical_func(spiReceivedIrqCallback)(uint8_t *data, size_t len)
{
  //digitalWrite(22, HIGH);
  
  //Serial.printf("rx : %d\n", len);
  //for (int i = 0; i < len; i++) {
  //  Serial.printf("  0x%02x\n", data[i]);
  //}

  if (len == 0) return;
  //xfer_count_ += len;

  uint8_t * dataend = data + len;
  while (1) {
    switch (spiState_)
    {
    case SPI_STATE_SYNC1:
      //Serial.printf("SYNC1\n");
      spiState_ = (*data == SPI_SYNC1)? SPI_STATE_SYNC2 : SPI_STATE_SYNC1;
      data++;
      break;
    case SPI_STATE_SYNC2:
      //Serial.printf("SYNC2\n");
      spiState_ = (*data == SPI_SYNC2)? SPI_STATE_CMD : SPI_STATE_SYNC1;
      data++;
      break;
    case SPI_STATE_CMD:
      //Serial.printf("CMD 0x%02x\n", *data);
      spiCmd_ = *data;
      data++;

      // Command Check
      switch (spiCmd_)
      {
      case SPI_CMD_NONE        :
      case SPI_CMD_GET_STATUS  :
      case SPI_CMD_START_FRAME :
      case SPI_CMD_SET_DATA    :
      case SPI_CMD_PING        :
      case SPI_CMD_OB_LED_ON   :
      case SPI_CMD_OB_LED_OFF  :
      case SPI_CMD_SET_ID_DIR0 :
      case SPI_CMD_SET_ID_DIR1 :
      case SPI_CMD_HARD_RESET  :
        // valid command
        spiState_ = SPI_STATE_OPT1;
        break;
      default:
        // unknown command
        spiState_ = SPI_SYNC1;
        spiResponseFlag_ = SPI_RSP_ERROR;
        break;
      }

      spiCrc_ = 0xFFFF;
      calc_crc16(SPI_SYNC1);
      calc_crc16(SPI_SYNC2);
      calc_crc16(spiCmd_);

      break;
    case SPI_STATE_OPT1:
      //Serial.printf("OPT1 0x%02x\n", *data);
      spiOpt1_ = *data;
      data++;
      spiState_ = SPI_STATE_OPT2;
      calc_crc16(spiOpt1_);
      break;
    case SPI_STATE_OPT2:
      //Serial.printf("OPT2 0x%02x\n", *data);
      spiOpt2_ = *data;
      data++;
      spiState_ = SPI_STATE_OPT3;
      calc_crc16(spiOpt2_);
      break;
    case SPI_STATE_OPT3:
      //Serial.printf("OPT3 0x%02x\n", *data);
      spiOpt3_ = *data;
      data++;
      spiState_ = SPI_STATE_OPT4;
      calc_crc16(spiOpt3_);
      break;
    case SPI_STATE_OPT4:
      //Serial.printf("OPT4 0x%02x\n", *data);
      spiOpt4_ = *data;
      data++;
      if ( (spiOpt1_ != (uint8_t)~spiOpt3_) || (spiOpt2_ != (uint8_t)~spiOpt4_) ) {
        //Serial.printf("OPT parity check error.\n");
        spiState_ = SPI_STATE_SYNC1;
        spiResponseFlag_ = SPI_RSP_ERROR;
      } else if (spiCmd_ == SPI_CMD_SET_DATA) {
        spiState_ = SPI_STATE_DATA;
        spiDataBlockSize_ = (uint)spiOpt2_ << 8 | (uint)spiOpt1_;
        calc_crc16(spiOpt4_);
        //Serial.printf("spiDataBlockSize_ = %d\n", spiDataBlockSize_);
      } else {
        spiState_ = SPI_STATE_CRC1;
        calc_crc16(spiOpt4_);
      }
      break;
    case SPI_STATE_DATA:
    {
      if (!buffer_.getWriteReady()) {
        //Serial.printf("Write buffer overflow\n");
        // The buffer is currently in I2C transfer standby. Discard data.
        spiState_ = SPI_STATE_SYNC1;
      }

      uint wrSize = MIN3((dataend - data), wrBufferAvail_, spiDataBlockSize_);

      memcpy(wrBufferPtr_, data, wrSize);
      for (int i = 0; i < wrSize; i++) {
        calc_crc16(*data);
        data++;
      }
      
      wrBufferPtr_ += wrSize;
      wrBufferAvail_ -= wrSize;
      spiDataBlockSize_ -= wrSize;
      
      if (wrBufferAvail_ == 0) {
        //Serial.printf("Set next buffer to current buffer.\n");
        buffer_.nextWriteBuffer();
        wrBufferPtr_   = buffer_.getWriteBufferPtr();
        wrBufferAvail_ = BUFFER_SIZE;
      }
      if (spiDataBlockSize_ == 0) {
        //Serial.printf("Done data state\n");
        spiState_ = SPI_STATE_CRC1;
      }
    } break;
    case SPI_STATE_CRC1:
      //Serial.printf("CRC1 0x%02x\n", *data);
      spiCrc1_ = *data;
      data++;
      spiState_ = SPI_STATE_CRC2;
      break;
    case SPI_STATE_CRC2:
    {
      //Serial.printf("CRC2 0x%02x\n", *data);
      spiCrc2_ = *data;
      data++;

      uint16_t crc = (uint16_t)spiCrc2_ << 8 | (uint16_t)spiCrc1_;
      //Serial.printf("CRC = 0x%04x\n", crc);
      if (spiCrc_ != crc) {
        //Serial.printf("CRC Check Error 0x%04x != 0x%04x\n", spiCrc_, crc);
        spiState_ = SPI_STATE_SYNC1;
        spiResponseFlag_ = SPI_RSP_ERROR;
        break;
      }

      switch (spiCmd_)
      {
      case SPI_CMD_GET_STATUS:
        //Serial.printf("run command SPI_CMD_GET_STATUS\n");
        spiResponseFlag_ = SPI_RSP_GET_STATUS;
        break;
      case SPI_CMD_START_FRAME:
        //Serial.printf("run command SPI_CMD_START_FRAME\n");
        wrBufferPtr_   = buffer_.getWriteBufferPtr();
        wrBufferAvail_ = BUFFER_SIZE;
        break;
      case SPI_CMD_PING:
        //Serial.printf("run command SPI_CMD_PING\n");
        spiResponseFlag_ = SPI_RSP_PING;
        break;
      case SPI_CMD_OB_LED_ON:
        //Serial.printf("run command SPI_CMD_OB_LED_ON\n");
        ob_led_on_ = true;
        break;
      case SPI_CMD_OB_LED_OFF:
        //Serial.printf("run command SPI_CMD_OB_LED_OFF\n");
        ob_led_on_ = false;
        break;
      case SPI_CMD_SET_ID_DIR0:
        //Serial.printf("run command SPI_CMD_SET_ID_DIR0\n");
        ssd1306mpio_.setIdDir(false);
        break;
      case SPI_CMD_SET_ID_DIR1:
        //Serial.printf("run command SPI_CMD_SET_ID_DIR1\n");
        ssd1306mpio_.setIdDir(true);
        break;
      case SPI_CMD_HARD_RESET:
        //Serial.printf("run command SPI_CMD_HARD_RESET\n");
        watchdog_enable(1, 1);
        while(1);
        break;
      default: break;
      }

      spiState_ = SPI_STATE_SYNC1;

      break;
    }
    default:
      spiState_ = SPI_STATE_SYNC1;
      break;
    }

    if (data >= dataend) {
      break;
    }
  }
  
  //digitalWrite(22, LOW);
}

//void spiSentIrqCallback()
void __time_critical_func(spiSentIrqCallback)()
{
  //Serial.printf("tx\n");

  switch (spiResponseFlag_)
  {
  case SPI_RSP_GET_STATUS:
  {
    uint8_t spibusy = (!buffer_.getWriteReady())? SPI_RSP_DATA_BUSY : 0x00;
    spiTxBuffer_ = SPI_TXDATA_VALID_FLAG | ((spibusy) & 0x7F);
  } break;
  case SPI_RSP_PING:
  {
    spiTxBuffer_ = SPI_TXDATA_VALID_FLAG | ((SPI_RSP_DATA_PING) & 0x7F);
  } break;
  case SPI_RSP_ERROR:
  {
    spiTxBuffer_ = SPI_TXDATA_VALID_FLAG | ((SPI_RSP_DATA_ERROR) & 0x7F);
  } break;
  case SPI_RSP_NONE:
  default:
    spiTxBuffer_ = 0;
    break;
  }
  spiResponseFlag_ = SPI_RSP_NONE;

  //Serial.printf("  %02x\n", spiTxBuffer_);
  SPISlave.setData((uint8_t*)&spiTxBuffer_, sizeof(spiTxBuffer_));
}
