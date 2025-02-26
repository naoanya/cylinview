/**********************************************************************/
/**
 * @brief  Pico SPI2I2C Bridge Controller
 * @author naoa
 */
/**********************************************************************/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Include files
 *----------------------------------------------------------------------
 */
#include <cstdint>
#include <SPI.h>

#include "spi_i2c_bridge.hpp"

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Definitions
 *----------------------------------------------------------------------
 */

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

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Definitions
 *----------------------------------------------------------------------
 */

static SPISettings spisettings(3000000, MSBFIRST, SPI_MODE0);
static uint16_t   crc16_lu_table[256]; 
static const uint16_t crc16_polynomial = 0x1021;

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Function definitions
 *----------------------------------------------------------------------
 */

SpiI2cBridge::SpiI2cBridge()
{

}

SpiI2cBridge::~SpiI2cBridge()
{

}

void
SpiI2cBridge::init(
    int pin_spi0_rx,
    int pin_spi0_cs,
    int pin_spi0_sck,
    int pin_spi0_tx,
    int pin_spi1_rx,
    int pin_spi1_cs,
    int pin_spi1_sck,
    int pin_spi1_tx
) {
    calc_crc16_lookup_table();

    SPI.setRX(pin_spi0_rx);
    SPI.setCS(pin_spi0_cs);
    SPI.setSCK(pin_spi0_sck);
    SPI.setTX(pin_spi0_tx);
    SPI.begin(true);
    SPI1.setRX(pin_spi1_rx);
    SPI1.setCS(pin_spi1_cs);
    SPI1.setSCK(pin_spi1_sck);
    SPI1.setTX(pin_spi1_tx);
    SPI1.begin(true);
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Function definitions
 *----------------------------------------------------------------------
 */

bool
SpiI2cBridge::waitReady(int id)
{
    uint32_t retry = 0xFFFFF;
    while (retry > 0) {
        sendCommand(id, SPI_CMD_GET_STATUS);
        uint8_t status = receiveResponse(id);
        if ((status & SPI_RSP_DATA_BUSY) == 0) {
            // receiver is ready.
            break;
        }
        retry--;
    }
    return (retry != 0);
}

bool
SpiI2cBridge::sendPing(int id) {
  sendCommand(id, SPI_CMD_PING);
  return ((receiveResponse(id) & 0x7F) == SPI_RSP_DATA_PING);
}

void
SpiI2cBridge::sendSetLED(int id, bool on) {
    sendCommand(id, (on)? SPI_CMD_OB_LED_ON : SPI_CMD_OB_LED_OFF);
}

void
SpiI2cBridge::sendSetIDDirection(int id, bool dir) {
    sendCommand(id, (dir)? SPI_CMD_SET_ID_DIR0 : SPI_CMD_SET_ID_DIR1);
}

void
SpiI2cBridge::sendHardReset(int id) {
    sendCommand(id, SPI_CMD_HARD_RESET);
}

bool
SpiI2cBridge::sendFrameDataParallel(uint8_t* buffer, size_t size)
{
    uint16_t blocksize = size / SIB_CHANNELS;
    uint8_t opt1 = blocksize >> 0;
    uint8_t opt2 = blocksize >> 8;
    uint8_t cmdbuf[9] = { SPI_SYNC1, SPI_SYNC2, SPI_CMD_SET_DATA, opt1, opt2, (uint8_t)~opt1, (uint8_t)~opt2, 0x00, 0x00 };

    // Calculate crc
    uint16_t basecrc16 = calc_crc16(cmdbuf, sizeof(cmdbuf) - 2);
    uint16_t crc16[SIB_CHANNELS];
    for (int id = 0; id < SIB_CHANNELS; id++) {
        crc16[id] = calc_crc16(buffer + (blocksize * id), blocksize, basecrc16);
    }

    // Wait device ready.
    for (int id = 0; id < SIB_CHANNELS; id++) {
        if (!waitReady(id)) {
            return false;
        }
    }

    // Send start frame command
    for (int id = 0; id < SIB_CHANNELS; id++) {
        sendCommand(id, SPI_CMD_START_FRAME);
    }

    // Send data command header
    for (int id = 0; id < SIB_CHANNELS; id++) {
        transfer(id, cmdbuf, NULL, sizeof(cmdbuf) - 2);
    }

    // Send data async
    for (int id = 0; id < SIB_CHANNELS; id++) {
        transferAsync(id, buffer + (blocksize * id), NULL, blocksize);
    }

    // Wait send data async
    for (int id = 0; id < SIB_CHANNELS; id++) {
        transferAsynEnd(id);
    }

    #if 1
    // Super Dirty Workaround
    uint8_t tmpbuffer[32];
    memset(tmpbuffer, 0, sizeof(tmpbuffer));
    for (int id = 0; id < SIB_CHANNELS; id++) {
      transfer(id, tmpbuffer, NULL, sizeof(tmpbuffer));
    }
    #endif

    // Send crc
    for (int id = 0; id < SIB_CHANNELS; id++) {
        uint8_t buf[2];
        buf[0] = (uint8_t)(crc16[id] >> 0);
        buf[1] = (uint8_t)(crc16[id] >> 8);
        transfer(id, buf, NULL, sizeof(buf));
    }

    return true;
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Function definitions
 *----------------------------------------------------------------------
 */

void
SpiI2cBridge::sendCommand(int id, uint8_t cmd, uint8_t opt1, uint8_t opt2) {
  uint8_t buffer[9] = { SPI_SYNC1, SPI_SYNC2, cmd, opt1, opt2, (uint8_t)~opt1, (uint8_t)~opt2, 0x00, 0x00 };
  uint16_t crc16 = calc_crc16(buffer, sizeof(buffer) - 2);
  buffer[7] = (uint8_t)(crc16 >> 0);
  buffer[8] = (uint8_t)(crc16 >> 8);
  transfer(id, buffer, NULL, sizeof(buffer));
}

uint8_t
SpiI2cBridge::receiveResponse(int id) {
  int retry = 0x20;
  while (retry) {
    uint8_t data = SPI_CMD_NONE;
    transfer(id, &data, &data, 1);
    if (data & SPI_TXDATA_VALID_FLAG) {
      return data;
    }
    retry--;
  }
  Serial.printf("SPI failed to receive data %d\n", id);
  return 0;
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Function definitions
 *----------------------------------------------------------------------
 */

void
SpiI2cBridge::transferAsync(int id, uint8_t* txbuffer, uint8_t* rxbuffer, size_t size) {
  SPIClassRP2040* spi = (id == 0) ? &SPI : &SPI1;

  spi->beginTransaction(spisettings);
  spi->transferAsync(txbuffer, rxbuffer, size);
}

void
SpiI2cBridge::transferAsynEnd(int id) {
  SPIClassRP2040* spi = (id == 0) ? &SPI : &SPI1;

  while (!spi->finishedAsync()) {}
  spi->endTransaction();
}

void
SpiI2cBridge::transfer(int id, uint8_t* txbuffer, uint8_t* rxbuffer, size_t size) {
  SPIClassRP2040* spi = (id == 0) ? &SPI : &SPI1;

  spi->beginTransaction(spisettings);
  spi->transferAsync(txbuffer, rxbuffer, size);
  while (!spi->finishedAsync()) {}
  spi->endTransaction();
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Function definitions
 *----------------------------------------------------------------------
 */

void
SpiI2cBridge::calc_crc16_lookup_table(void)
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

uint16_t
SpiI2cBridge::calc_crc16(uint8_t * data, size_t size, uint16_t crc)
{
    for (int i = 0; i < size; i++) {
        crc = (crc << 8) ^ crc16_lu_table[((crc >> 8) ^ data[i]) & 0xFF];
    }
    return crc;
}
