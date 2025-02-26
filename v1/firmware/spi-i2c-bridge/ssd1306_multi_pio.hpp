/**********************************************************************/
/**
 * @brief  SSD1306 controller supported multi pio channels
 * @author naoa
 */
/**********************************************************************/
#pragma once
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Include files
 *----------------------------------------------------------------------
 */
#if !defined(ARDUINO)
#error "Not supported without arduino ide."
#endif
#include <Arduino.h>
#include <cstdint>

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Definitions
 *----------------------------------------------------------------------
 */

#define SSD1306MPIO_EXTERNALVCC         (1)
#define SSD1306MPIO_SWITCHCAPVCC        (2)

#define SSD1306MPIO_HORIZONTAL_ADDRESSING_MODE  (0)
#define SSD1306MPIO_VERTICAL_ADDRESSING_MODE    (1)
#define SSD1306MPIO_PAGE_ADDRESSING_MODE        (2)

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Class definitions
 *----------------------------------------------------------------------
 */

class SSD1306MultiPIO
{
public:
    explicit SSD1306MultiPIO();
    virtual ~SSD1306MultiPIO();

public:
    void init(
        uint8_t i2cAddr = 0x3C,
        int ch = 8,
        int pinBase = 0,
        int vccstate = SSD1306MPIO_SWITCHCAPVCC
    );

    void setIdDir(bool dir);

public:
    void displayOn(void);
    void displayOff(void);
    void setContrast(uint8_t contrast);
    void setInvertMode(void);
    void setNormalMode(void);
    void flipHorizontal(int mode);
    void flipVertical(int mode);
    void setStartLine(uint8_t line);

public:
    void writeFrame(int id, uint8_t * buffer, size_t size);
    void writeFrameMulti(uint8_t * buffer);

private:
    inline int send_cmd_all(uint8_t cmd);
    inline int send_cmd_all(uint8_t * cmds, size_t size);
    inline int send_dat_all(uint8_t data);
    inline int send_dat_all(uint8_t * data, size_t size);
    inline int send_all(uint8_t * data, size_t size, bool cmd = true);

    inline int send_cmd(int id, uint8_t cmd);
    inline int send_cmd(int id, uint8_t * cmds, size_t size);
    inline int send_dat(int id, uint8_t data);
    inline int send_dat(int id, uint8_t * data, size_t size);
           int send(int id, uint8_t * data, size_t size, bool cmd = true);

           int send_parallel(uint8_t * data, size_t size, bool cmd = true);

private:
    inline void multi_pio_i2c_start(void);
    inline void multi_pio_i2c_stop(void);
    inline void multi_pio_i2c_repstart(void);
    inline void multi_pio_i2c_rx_enable(bool en);
    inline bool multi_pio_i2c_check_error(void);
    inline void multi_pio_i2c_resume_after_error(void);
    inline void multi_pio_i2c_put16(uint16_t data);
    inline void multi_pio_i2c_put_or_err(uint16_t data);
    inline void multi_pio_i2c_get(uint8_t * buffer);
    inline void multi_pio_i2c_wait_idle(void);
    inline bool multi_pio_sm_is_tx_fifo_full(void);

private:
    uint8_t i2cAddr_;
    int ch_;
    int pinBase_;
    bool inited_ = false;
    uint8_t contrast_;
    bool idDir_ = true;
};
