/**********************************************************************/
/**
 * @brief  Serial Command
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

#define SERIALCMD_MAX_COMMAND_LEN   (256)
#define SERIALCMD_MAX_PARAMS_NUM    (16)

#define SERIALCMD_ONE_PARAM_LEN     (SERIALCMD_MAX_COMMAND_LEN / (SERIALCMD_MAX_PARAMS_NUM + 1 /* Command */) )

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Forword Definitions
 *----------------------------------------------------------------------
 */

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Namespace
 *----------------------------------------------------------------------
 */

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Class Definitions
 *----------------------------------------------------------------------
 */

class SerialCmd
{
public:
    SerialCmd(HardwareSerial * serial);
    ~SerialCmd();

    int loop(void);
    char * getCmd(void);
    char * getParam(int index);
    bool checkCmd(const char * cmd);
    bool checkParam(int index, const char * param);

    void setSerial(HardwareSerial * serial);
    HardwareSerial * serial(void);

    static int stringToInt(const char * str);
    static char stringToChar(const char * str);
    static float stringToFloat(const char * str);
    static uint64_t stringToUInt64(const char * str);
    static uint32_t stringHexToUInt(const char * str);

private:
    void clearBuffer(void);
    void getArg(uint8_t *dst, int size, int index);

private:
    HardwareSerial * serial_ = NULL;

    uint8_t buffer_[SERIALCMD_MAX_COMMAND_LEN];
    uint32_t buffer_i_ = 0;

    char cmd_[SERIALCMD_ONE_PARAM_LEN];
    char param_[SERIALCMD_MAX_PARAMS_NUM][SERIALCMD_ONE_PARAM_LEN];
    int param1int_ = 0;
};
