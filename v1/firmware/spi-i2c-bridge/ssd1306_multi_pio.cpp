/**********************************************************************/
/**
 * @brief  SSD1306 controller supported multi pio channels
 * @author naoa
 */
/**********************************************************************/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Include files
 *----------------------------------------------------------------------
 */
#include "pico/stdlib.h"
#include "pio_i2c.h"
#include "ssd1306_multi_pio.hpp"

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Definitions - Debug
 *----------------------------------------------------------------------
 */

#define DP_PREFIX_STR   "SSD1306MPIO: "

//#define DP__(...)           Serial.printf("DEBUG: " DP_PREFIX_STR __VA_ARGS__)
#define DP__(...)

#define DP_INFO__(...)      Serial.printf("INFO: " DP_PREFIX_STR __VA_ARGS__)
//#define DP_INFO__(...)

#define DP_ERROR__(...)     Serial.printf("ERROR: " DP_PREFIX_STR __VA_ARGS__)
//#define DP_ERROR__(...)

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Definitions
 *----------------------------------------------------------------------
 */

#define SSD1306MPIO_MAX_CH              (NUM_PIOS * NUM_PIO_STATE_MACHINES)

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Definitions
 *----------------------------------------------------------------------
 */

#define SSD1306MPIO_SETLOWCOLUMN        (0x00)
#define SSD1306MPIO_SETHIGHCOLUMN       (0x10)
#define SSD1306MPIO_MEMORYMODE          (0x20)
#define SSD1306MPIO_COLUMNADDR          (0x21)
#define SSD1306MPIO_PAGEADDR            (0x22)
#define SSD1306MPIO_DEACTIVATE_SCROLL   (0x2E)
#define SSD1306MPIO_SETSTARTLINE        (0x40)
#define SSD1306MPIO_SETCONTRAST         (0x81)
#define SSD1306MPIO_CHARGEPUMP          (0x8D)
#define SSD1306MPIO_SEGREMAP            (0xA0)
#define SSD1306MPIO_DISPLAYALLON_RESUME (0xA4)
#define SSD1306MPIO_DISPLAYALLON        (0xA5)
#define SSD1306MPIO_NORMALDISPLAY       (0xA6)
#define SSD1306MPIO_INVERTDISPLAY       (0xA7)
#define SSD1306MPIO_SETMULTIPLEX        (0xA8)
#define SSD1306MPIO_DISPLAYOFF          (0xAE)
#define SSD1306MPIO_DISPLAYON           (0xAF)
#define SSD1306MPIO_COMSCANINC          (0xC0)
#define SSD1306MPIO_COMSCANDEC          (0xC8)
#define SSD1306MPIO_SETDISPLAYOFFSET    (0xD3)
#define SSD1306MPIO_SETDISPLAYCLOCKDIV  (0xD5)
#define SSD1306MPIO_SETPRECHARGE        (0xD9)
#define SSD1306MPIO_SETCOMPINS          (0xDA)
#define SSD1306MPIO_SETVCOMDETECT       (0xDB)

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Definitions
 *----------------------------------------------------------------------
 */

static const PIO piolist0[SSD1306MPIO_MAX_CH] = { pio0, pio0, pio0, pio0, pio1, pio1, pio1, pio1 };
static const uint smlist0[SSD1306MPIO_MAX_CH] = { 0, 1, 2, 3, 0, 1, 2, 3 };
static const PIO piolist1[SSD1306MPIO_MAX_CH] = { pio1, pio1, pio1, pio1, pio0, pio0, pio0, pio0 };
static const uint smlist1[SSD1306MPIO_MAX_CH] = { 3, 2, 1, 0, 3, 2, 1, 0 };

static PIO  const * piolistptr_ = piolist0;
static uint const * smlistptr_ = smlist0;

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Function definitions
 *----------------------------------------------------------------------
 */

SSD1306MultiPIO::SSD1306MultiPIO() :
    inited_(false)
{
}

SSD1306MultiPIO::~SSD1306MultiPIO()
{
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Function definitions
 *----------------------------------------------------------------------
 */

void
SSD1306MultiPIO::init(
    uint8_t i2cAddr,
    int ch,
    int pinBase,
    int vccstate
) {
    if (ch > SSD1306MPIO_MAX_CH || ch < 1) {
        DP_ERROR__("Unsupported channels = %d\n", ch);
        return;
    }

    i2cAddr_ = i2cAddr;
    ch_ = ch;
    pinBase_ = pinBase;
    contrast_ = 0x8F;

    uint offset0 = pio_add_program(pio0, &i2c_program);
    uint offset1 = (ch_ > NUM_PIO_STATE_MACHINES)? pio_add_program(pio1, &i2c_program) : 0;
    for (int id = 0; id < ch_; id++) {
        PIO pio = (id < NUM_PIO_STATE_MACHINES)? pio0 : pio1;
        uint sm = id % NUM_PIO_STATE_MACHINES; 
        uint offset = (id < NUM_PIO_STATE_MACHINES)? offset0 : offset1;
        int pinSDA = pinBase_ + id * 2;
        int pinSCL = pinSDA + 1;
        DP_INFO__("pio init : id = %d, sm = %d, offset = %d, pinSDA = %d, pinSCL = %d\n", id, sm, offset, pinSDA, pinSCL);
        i2c_program_init(pio, sm, offset, pinSDA, pinSCL);
    }

    uint8_t precharge = (vccstate == SSD1306MPIO_EXTERNALVCC) ? 0x22 : 0xF1;
    uint8_t initdata_128_32[] = {
        SSD1306MPIO_DISPLAYOFF,
        SSD1306MPIO_SETDISPLAYCLOCKDIV, 0x80,
        SSD1306MPIO_SETMULTIPLEX, 32 - 1,
        SSD1306MPIO_SETDISPLAYOFFSET, 0x00,
        SSD1306MPIO_SETSTARTLINE | 0x00,
        SSD1306MPIO_CHARGEPUMP, 0x14,
        SSD1306MPIO_SEGREMAP | 0x1,
        SSD1306MPIO_COMSCANDEC,
        SSD1306MPIO_MEMORYMODE, SSD1306MPIO_HORIZONTAL_ADDRESSING_MODE,
        SSD1306MPIO_SETCOMPINS, 0x02,
        SSD1306MPIO_SETCONTRAST, contrast_,
        SSD1306MPIO_SETPRECHARGE, precharge,
        SSD1306MPIO_SETVCOMDETECT, 0x40,
        SSD1306MPIO_DISPLAYALLON_RESUME,
        SSD1306MPIO_NORMALDISPLAY,
        SSD1306MPIO_DEACTIVATE_SCROLL,
        SSD1306MPIO_DISPLAYON
    };

    for (int i = 0; i < sizeof(initdata_128_32); i++) {
        send_cmd_all(initdata_128_32[i]);
    }

    uint8_t tmpbuffer[128 * 32 / 8];
    memset(tmpbuffer, 0, sizeof(tmpbuffer));
    for (int id = 0; id < ch_; id++) {
        writeFrame(id, tmpbuffer, sizeof(tmpbuffer));
    }

    inited_ = true;
}

void
SSD1306MultiPIO::setIdDir(bool dir)
{
    idDir_ = dir;

    if (dir) {
        piolistptr_ = piolist1;
        smlistptr_ = smlist1;
    } else {
        piolistptr_ = piolist0;
        smlistptr_ = smlist0;
    }
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Function definitions
 *----------------------------------------------------------------------
 */

void
SSD1306MultiPIO::displayOn(void)
{
    send_cmd_all(SSD1306MPIO_DISPLAYON);
}

void
SSD1306MultiPIO::displayOff(void)
{
    send_cmd_all(SSD1306MPIO_DISPLAYOFF);
}

void
SSD1306MultiPIO::setContrast(uint8_t contrast)
{
    uint8_t buffer[] {
        SSD1306MPIO_SETCONTRAST,
        contrast
    };
    send_cmd_all(buffer, sizeof(buffer));
}

void
SSD1306MultiPIO::setInvertMode(void)
{
    send_cmd_all(SSD1306MPIO_INVERTDISPLAY);
}

void
SSD1306MultiPIO::setNormalMode(void)
{
    send_cmd_all(SSD1306MPIO_NORMALDISPLAY);
}

void
SSD1306MultiPIO::flipHorizontal(int mode)
{
    send_cmd_all(SSD1306MPIO_SEGREMAP | ((mode)? 0 : 1));
}

void
SSD1306MultiPIO::flipVertical(int mode)
{
    send_cmd_all((mode)? SSD1306MPIO_COMSCANINC : SSD1306MPIO_COMSCANDEC);
}

void
SSD1306MultiPIO::setStartLine(uint8_t line)
{
    send_cmd_all(SSD1306MPIO_SETSTARTLINE | (line & 0x3F));
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Function definitions
 *----------------------------------------------------------------------
 */

void
SSD1306MultiPIO::writeFrame(int id, uint8_t * buffer, size_t size)
{
    uint8_t cmdbuffer[] {
        SSD1306MPIO_PAGEADDR,
        0x00,
        0xFF,
        SSD1306MPIO_COLUMNADDR,
        0x00,
        128 - 1
    };

    send_cmd(id, cmdbuffer, sizeof(cmdbuffer));
    send_dat(id, buffer, size);
}

void
SSD1306MultiPIO::writeFrameMulti(uint8_t * buffer)
{
    uint oneFrameBytes = ((128 * 32) / 8 /* bit */);

    uint8_t * bufptrs[SSD1306MPIO_MAX_CH];
    for (int id = 0; id < ch_; id++) {
        bufptrs[id] = buffer + (id * oneFrameBytes);
    }

    //
    // Send frame address.
    //

    uint8_t cmdbuffer[] {
        SSD1306MPIO_PAGEADDR,   0x00, 0xFF,
        SSD1306MPIO_COLUMNADDR, 0x00, 128 - 1
    };
    send_cmd_all(cmdbuffer, sizeof(cmdbuffer));

    //
    // Send frame data in parallel.
    //

    uint len;

    multi_pio_i2c_start();
    multi_pio_i2c_rx_enable(false);

    multi_pio_i2c_put16((i2cAddr_ << 2) | 1u);

    len = 1;
    while (len && !multi_pio_i2c_check_error()) {
        if (!multi_pio_sm_is_tx_fifo_full()) {
            --len;
            multi_pio_i2c_put_or_err(
                (((uint8_t)0x40) << PIO_I2C_DATA_LSB)
                | ((len == 0) << PIO_I2C_FINAL_LSB)
                | 1u
            );
        }
    }
    
    len = oneFrameBytes;
    while (len && !multi_pio_i2c_check_error()) {
        if (!multi_pio_sm_is_tx_fifo_full()) {
            --len;
            for (int id = 0; id < ch_; id++) {
                pio_i2c_put_or_err(piolistptr_[id], smlistptr_[id], 
                    (*bufptrs[id] << PIO_I2C_DATA_LSB)
                    | ((len == 0) << PIO_I2C_FINAL_LSB)
                    | 1u
                );
                bufptrs[id]++;
            }
        }
    }
    
    multi_pio_i2c_stop();
    multi_pio_i2c_wait_idle();
    
    if (multi_pio_i2c_check_error()) {
        multi_pio_i2c_resume_after_error();
        multi_pio_i2c_stop();
    }
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Function definitions
 *----------------------------------------------------------------------
 */

int
SSD1306MultiPIO::send_cmd_all(uint8_t cmd)
{
    return send_cmd_all(&cmd, 1);
}

int
SSD1306MultiPIO::send_cmd_all(uint8_t * cmds, size_t size)
{
    return send_all(cmds, size, true);
}

int
SSD1306MultiPIO::send_dat_all(uint8_t data)
{
    return send_cmd_all(&data, 1);
}

int
SSD1306MultiPIO::send_dat_all(uint8_t * data, size_t size)
{
    return send_all(data, size, true);
}

int
SSD1306MultiPIO::send_all(uint8_t * data, size_t size, bool cmd)
{
#if 0
    int ret = 0;
    for (int id = 0; id < ch_; id++) {
        ret |= send(id, data, size, cmd);
    }
    return ret;
#else
    return send_parallel(data, size, cmd);
#endif
}

int
SSD1306MultiPIO::send_cmd(int id, uint8_t cmd)
{
    return send_cmd(id, &cmd, 1);
}

int
SSD1306MultiPIO::send_cmd(int id, uint8_t * cmds, size_t size)
{
    return send(id, cmds, size, true);
}

int
SSD1306MultiPIO::send_dat(int id, uint8_t data)
{
    return send_dat(id, &data, 1);
}

int
SSD1306MultiPIO::send_dat(int id, uint8_t * data, size_t size)
{
    return send(id, data, size, false);
}

int
SSD1306MultiPIO::send(int id, uint8_t * data, size_t size, bool cmd)
{
    int err = 0;
    uint len;
    
    //DP_INFO__("send : %d 0x%02x (%d)(%d)\n", id, *data, size, cmd);

    PIO pio = piolistptr_[id];
    uint sm = smlistptr_[id];

    pio_i2c_start(pio, sm);
    pio_i2c_rx_enable(pio, sm, false);
    
    pio_i2c_put16(pio, sm, (i2cAddr_ << 2) | 1u);

    len = 1;
    while (len && !pio_i2c_check_error(pio, sm)) {
        if (!pio_sm_is_tx_fifo_full(pio, sm)) {
            --len;
            pio_i2c_put_or_err(pio, sm, 
                (((cmd)? (uint8_t)0x00 : (uint8_t)0x40) << PIO_I2C_DATA_LSB)
                | ((len == 0) << PIO_I2C_FINAL_LSB)
                | 1u
            );
        }
    }
    
    len = size;
    while (len && !pio_i2c_check_error(pio, sm)) {
        if (!pio_sm_is_tx_fifo_full(pio, sm)) {
            --len;
            pio_i2c_put_or_err(pio, sm, 
                (*data++ << PIO_I2C_DATA_LSB)
                | ((len == 0) << PIO_I2C_FINAL_LSB)
                | 1u
            );
        }
    }
    
    pio_i2c_stop(pio, sm);
    pio_i2c_wait_idle(pio, sm);
    
    if (pio_i2c_check_error(pio, sm)) {
        err = -1;
        pio_i2c_resume_after_error(pio, sm);
        pio_i2c_stop(pio, sm);
    }

    return err;
}

int
SSD1306MultiPIO::send_parallel(uint8_t * data, size_t size, bool cmd)
{
    int err = 0;
    uint len;

    multi_pio_i2c_start();
    multi_pio_i2c_rx_enable(false);

    multi_pio_i2c_put16((i2cAddr_ << 2) | 1u);

    len = 1;
    while (len && !multi_pio_i2c_check_error()) {
        if (!multi_pio_sm_is_tx_fifo_full()) {
            --len;
            multi_pio_i2c_put_or_err(
                (((cmd)? (uint8_t)0x00 : (uint8_t)0x40) << PIO_I2C_DATA_LSB)
                | ((len == 0) << PIO_I2C_FINAL_LSB)
                | 1u
            );
        }
    }
    
    len = size;
    while (len && !multi_pio_i2c_check_error()) {
        if (!multi_pio_sm_is_tx_fifo_full()) {
            --len;
            multi_pio_i2c_put_or_err(
                (*data++ << PIO_I2C_DATA_LSB)
                | ((len == 0) << PIO_I2C_FINAL_LSB)
                | 1u
            );
        }
    }
    
    multi_pio_i2c_stop();
    multi_pio_i2c_wait_idle();
    
    if (multi_pio_i2c_check_error()) {
        err = -1;
        multi_pio_i2c_resume_after_error();
        multi_pio_i2c_stop();
    }

    return err;
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Function definitions
 *----------------------------------------------------------------------
 */

void
SSD1306MultiPIO::multi_pio_i2c_start(void)
{
    for (int id = 0; id < ch_; id++) {
        pio_i2c_start(piolistptr_[id], smlistptr_[id]);
    }
}

void
SSD1306MultiPIO::multi_pio_i2c_stop(void)
{
    for (int id = 0; id < ch_; id++) {
        pio_i2c_stop(piolistptr_[id], smlistptr_[id]);
    }
}

void
SSD1306MultiPIO::multi_pio_i2c_repstart(void)
{
    for (int id = 0; id < ch_; id++) {
        pio_i2c_repstart(piolistptr_[id], smlistptr_[id]);
    }
}

void
SSD1306MultiPIO::multi_pio_i2c_rx_enable(bool en)
{
    for (int id = 0; id < ch_; id++) {
        pio_i2c_rx_enable(piolistptr_[id], smlistptr_[id], en);
    }
}

bool
SSD1306MultiPIO::multi_pio_i2c_check_error(void)
{
    for (int id = 0; id < ch_; id++) {
        if (pio_i2c_check_error(piolistptr_[id], smlistptr_[id])) return true;
    }
    return false;
}

void
SSD1306MultiPIO::multi_pio_i2c_resume_after_error(void)
{
    for (int id = 0; id < ch_; id++) {
        pio_i2c_resume_after_error(piolistptr_[id], smlistptr_[id]);
    }
}

void
SSD1306MultiPIO::multi_pio_i2c_put16(uint16_t data)
{
    for (int id = 0; id < ch_; id++) {
        pio_i2c_put16(piolistptr_[id], smlistptr_[id], data);
    }
}

void
SSD1306MultiPIO::multi_pio_i2c_put_or_err(uint16_t data)
{
    for (int id = 0; id < ch_; id++) {
        pio_i2c_put_or_err(piolistptr_[id], smlistptr_[id], data);
    }
}

void
SSD1306MultiPIO::multi_pio_i2c_get(uint8_t * buffer)
{
    for (int id = 0; id < ch_; id++) {
        buffer[id] = pio_i2c_get(piolistptr_[id], smlistptr_[id]);
    }
}

void
SSD1306MultiPIO::multi_pio_i2c_wait_idle(void)
{
    for (int id = 0; id < ch_; id++) {
        pio_i2c_wait_idle(piolistptr_[id], smlistptr_[id]);
    }
}

// Returns true (full) if at least one of the fifo is full.
bool
SSD1306MultiPIO::multi_pio_sm_is_tx_fifo_full(void)
{
    for (int id = 0; id < ch_; id++) {
        if (pio_sm_is_tx_fifo_full(piolistptr_[id], smlistptr_[id])) return true;
    }
    return false;
}