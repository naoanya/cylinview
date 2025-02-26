/**********************************************************************/
/**
 * @brief  Pico SPI2I2C Bridge Controller
 * @author naoa
 */
/**********************************************************************/
#pragma once
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Include files
 *----------------------------------------------------------------------
 */
#include <cstdint>

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Definitions
 *----------------------------------------------------------------------
 */

#define SIB_CHANNELS      (2)

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Class definitions
 *----------------------------------------------------------------------
 */

class SpiI2cBridge
{
public:
    explicit SpiI2cBridge();
    virtual ~SpiI2cBridge();

public:
    void init(
        int pin_spi0_rx,
        int pin_spi0_cs,
        int pin_spi0_sck,
        int pin_spi0_tx,
        int pin_spi1_rx,
        int pin_spi1_cs,
        int pin_spi1_sck,
        int pin_spi1_tx
    );

public:
    bool waitReady(int id);
    bool sendPing(int id);
    void sendSetLED(int id, bool on);
    void sendSetIDDirection(int id, bool dir);
    void sendHardReset(int id);
    bool sendFrameDataParallel(uint8_t * buffer, size_t size);

public:
    void sendCommand(int id, uint8_t cmd, uint8_t opt1 = 0x55, uint8_t opt2 = 0x55);
    uint8_t receiveResponse(int id);

public:
    void transferAsync(int id, uint8_t * txbuffer, uint8_t * rxbuffer, size_t size);
    void transferAsynEnd(int id);
    void transfer(int id, uint8_t * txbuffer, uint8_t * rxbuffer, size_t size);

private:
    static void calc_crc16_lookup_table(void);
    static inline uint16_t calc_crc16(uint8_t * data, size_t size, uint16_t crc = 0xFFFF);
};
