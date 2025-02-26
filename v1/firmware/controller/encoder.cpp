/**********************************************************************/
/**
 * @brief  AS5048A Magnetic Encoder
 * @author naoa
 */
/**********************************************************************/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Include files
 *----------------------------------------------------------------------
 */
#include <cstdint>
#include <cmath>
#include "pio_spi.h"

#include "encoder.hpp"

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Definitions
 *----------------------------------------------------------------------
 */

static void pwm_pin_change_irq_handler(void);

static uint16_t readAngle(void);
static uint16_t calcParity(uint16_t value);
static uint16_t read(uint16_t regaddr);
static uint16_t write(uint16_t regaddr, uint16_t data);
static void piospi_xfer(uint8_t * txbuf, uint8_t * rxbuf, size_t size);

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Definitions
 *----------------------------------------------------------------------
 */

#define AS5048A_REG_ANGLE   (0x3FFF)

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Definitions
 *----------------------------------------------------------------------
 */

static int pin_pwm_;
static int pin_rx_;
static int pin_cs_;
static int pin_sck_;
static int pin_tx_;

static uint32_t pre_int_us_ = 0;
static uint32_t int_len_us_ = 0;
static uint32_t int_time_us_ = 0;

static uint32_t pwm_min_us_ = 4;
static uint32_t pwm_max_us_ = 904;
static uint32_t pwm_len_us_;

static pio_spi_inst_t spi = {
    .pio = pio0,
    .sm = 0
};

static uint16_t angleOffset_ = 0;

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Function definitions
 *----------------------------------------------------------------------
 */

void encoder_init_pwm(
    int pin_pwm
) {
    pin_pwm_ = pin_pwm;
    pinMode( pin_pwm_, INPUT );

    pwm_len_us_ = pwm_max_us_ - pwm_min_us_ + 1;

    pre_int_us_ = micros();
    attachInterrupt(digitalPinToInterrupt(pin_pwm_), pwm_pin_change_irq_handler, CHANGE);
}

void encoder_init_spi(
    int pin_rx,
    int pin_cs,
    int pin_sck,
    int pin_tx
) {
    pin_rx_  = pin_rx;
    pin_cs_  = pin_cs;
    pin_sck_ = pin_sck;
    pin_tx_  = pin_tx;

    pinMode( pin_cs_, OUTPUT );
    digitalWrite( pin_cs_, HIGH );

    uint offset = pio_add_program(spi.pio, &spi_cpha1_program);
    pio_spi_init(spi.pio, spi.sm, offset,
                 8,       // 8 bits per SPI frame
                 31.25f,  // 1 MHz @ 125 clk_sys
                 true,    // CPHA = 0
                 false,   // CPOL = 0
                 pin_sck,
                 pin_tx,
                 pin_rx
    );
}

float encoder_get_angle_pwm(void)
{
    int len = (int)int_len_us_;
    if (len > pwm_max_us_) len = pwm_max_us_;
    if (len < pwm_min_us_) len = pwm_min_us_;
    return( (float) (len - pwm_min_us_) / (float)pwm_len_us_) * (2.0 * M_PI);
}

uint32_t encoder_get_raw_data_pwm(void)
{
    // @TODO support angleOffset_
    return int_len_us_;
}

float encoder_get_angle_spi(void)
{
	return ((float)((readAngle() + angleOffset_) % 0x3FFF) / 0x4000) * (2.0 * M_PI);
}

uint32_t encoder_get_raw_data_spi(void)
{
    return (readAngle() + angleOffset_) % 0x3FFF;
}

void encoder_set_angle_offset(uint16_t offset)
{
    angleOffset_ = offset;
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Function definitions
 *----------------------------------------------------------------------
 */

static void pwm_pin_change_irq_handler(void)
{
    uint32_t cur_int_us = micros();
    if (!digitalRead(pin_pwm_)) {
        int_len_us_ = cur_int_us - pre_int_us_;
        int_time_us_ = pre_int_us_;
    }
    pre_int_us_ = cur_int_us;
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Function definitions
 *----------------------------------------------------------------------
 */

uint16_t readAngle(void)
{
	return read(AS5048A_REG_ANGLE);
}

uint16_t calcParity(uint16_t value)
{
	uint16_t count = 0;
	for (uint16_t i = 0; i < 16; i++) {
		if (value & 0x1) count++;
		value >>= 1;
	}
	return count & 0x1;
}

uint16_t read(uint16_t regaddr)
{
    uint16_t cmd = 0;
    cmd |= (1 << 14);
    cmd |= (regaddr << 0);
    cmd |= (calcParity(cmd) << 15);
    { uint16_t tmp = cmd; cmd = tmp >> 8;  cmd |= tmp << 8; }
    piospi_xfer((uint8_t*)&cmd, nullptr, sizeof(cmd));

    //delay(50);

    uint16_t res = 0;
    piospi_xfer(nullptr, (uint8_t*)&res, sizeof(res));
    { uint16_t tmp = res; res = tmp >> 8;  res |= tmp << 8; }

    #if 0
    bool parity = (res & (1 << 15));
    if (parity != calcParity(res)) {
        // @todo parity check failed
    }
    #endif

    #if 0
    if (res & (1 << 14)) {
        // @todo error bit avail
    }
    #endif

    return res & 0x3FFF;
}

uint16_t write(uint16_t regaddr, uint16_t data)
{
    uint16_t cmd = 0;
    cmd |= (0 << 14);
    cmd |= (regaddr << 0);
    cmd |= (calcParity(cmd) << 15);
    { uint16_t tmp = cmd; cmd = tmp >> 8;  cmd |= tmp << 8; }
    piospi_xfer((uint8_t*)&cmd, nullptr, sizeof(cmd));

    data &= 0x3FFF;
    data |= (calcParity(cmd) << 15);
    { uint16_t tmp = data; data = tmp >> 8;  data |= tmp << 8; }
    piospi_xfer((uint8_t*)&data, nullptr, sizeof(data));

    //delay(50);

    uint16_t res = 0;
    piospi_xfer(nullptr, (uint8_t*)&res, sizeof(res));
    { uint16_t tmp = res; res = tmp >> 8;  res |= tmp << 8; }

    #if 0
    bool parity = (res & (1 << 15));
    if (parity != calcParity(res)) {
        // @todo parity check failed
    }
    #endif

    #if 0
    if (res & (1 << 14)) {
        // @todo error bit avail
    }
    #endif

    return res & 0x3FFF;
}

static void piospi_xfer(uint8_t * txbuf, uint8_t * rxbuf, size_t size)
{
    digitalWrite( pin_cs_, LOW );
    if (txbuf == nullptr && rxbuf == nullptr) {
        // @TODO error
    } else if (txbuf == nullptr) {
        pio_spi_read8_blocking(&spi, rxbuf, size);
    } else if (rxbuf == nullptr) {
        pio_spi_write8_blocking(&spi, txbuf, size);
    } else {
        pio_spi_write8_read8_blocking(&spi, txbuf, rxbuf, size);
    }
    digitalWrite( pin_cs_, HIGH );
}
