/**********************************************************************/
/**
 * @brief  Serial Command
 * @author naoa
 */
/**********************************************************************/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Include files
 *----------------------------------------------------------------------
 */
#include <Arduino.h>
#include <string.h>
#include <errno.h>

#include "serialcmd.hpp"

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Namespace
 *----------------------------------------------------------------------
 */

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Method Implementation
 *----------------------------------------------------------------------
 */

SerialCmd::SerialCmd(HardwareSerial * serial) :
    serial_(serial)
{
    clearBuffer();
}

SerialCmd::~SerialCmd()
{

}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Method Implementation
 *----------------------------------------------------------------------
 */

int
SerialCmd::loop(void)
{
    if (serial_ == NULL) return 0;
    if (serial_->available() == 0) return 0;

    char c = serial_->read();
    if (c == '\n') {
        // Line feed detected
        getArg((uint8_t*)cmd_, sizeof(cmd_), 0);
        for (int i = 0; i < SERIALCMD_MAX_PARAMS_NUM; i++) {
            getArg((uint8_t*)param_[i], SERIALCMD_ONE_PARAM_LEN, i + 1);
        }
        clearBuffer();
        // command found
        return 1;
    } else {
        // Save into buffer
        buffer_[buffer_i_] = c;
        buffer_i_++;
        if (buffer_i_ >= sizeof(buffer_)) {
            // overwraped, reset buffer.
            Serial.println("SerialCmd: mycommand buffer overflow");
            clearBuffer();
        }
    }

    return 0;
}

char *
SerialCmd::getCmd(void)
{
    return cmd_;
}

char *
SerialCmd::getParam(int index)
{
    return param_[index];
}

bool
SerialCmd::checkCmd(const char * cmd)
{
    return strcmp(getCmd(), cmd) == 0;
}

bool
SerialCmd::checkParam(int index, const char * param)
{
    return strcmp(getParam(index), param) == 0;
}

void
SerialCmd::setSerial(HardwareSerial * serial)
{
    serial_ = serial;
}

HardwareSerial *
SerialCmd::serial(void)
{
    return serial_;
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Method Implementation
 *----------------------------------------------------------------------
 */

int
SerialCmd::stringToInt(const char * str)
{
    return String(str).toInt();
}

char
SerialCmd::stringToChar(const char * str)
{
    return String(str).toInt();
}

uint64_t
SerialCmd::stringToUInt64(const char * str)
{
  errno = 0;
  uint64_t ret = strtoull(str, NULL, 10);
  if (errno != 0) { ret = 0; }
  return ret;
}

float
SerialCmd::stringToFloat(const char * str)
{
    return String(str).toFloat();
}

uint32_t
SerialCmd::stringHexToUInt(const char * str)
{
    uint32_t val = 0;
    while (*str) {
        uint8_t byte = *str++; 
        if (byte >= '0' && byte <= '9') byte = byte - '0';
        else if (byte >= 'a' && byte <='f') byte = byte - 'a' + 10;
        else if (byte >= 'A' && byte <='F') byte = byte - 'A' + 10;    
        val = (val << 4) | (byte & 0xF);
    }
    return val;
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Method Implementation
 *----------------------------------------------------------------------
 */

void
SerialCmd::clearBuffer(void)
{
    buffer_i_ = 0;
    memset(buffer_, 0, sizeof(buffer_));
}

void
SerialCmd::getArg(uint8_t *dst, int size, int index)
{
    memset(dst, 0, size);

    uint8_t * start = NULL;
    uint8_t * end = NULL;

    int curIndex = 0;
    int bufptr = 0;
    while (1) {
        int pos;
        int startPos = 0;
        int endPos = 0;

        // Search start position
        pos = -1;
        while (1) {
            if (buffer_[bufptr] != ' ' && buffer_[bufptr] != '\0' && buffer_[bufptr] != '\n') { pos = bufptr; break; }
            bufptr++;
            if (bufptr >= sizeof(buffer_)) break;
        }
        if (pos == -1) {
            break;
        }
        startPos = pos;

        // Search end position
        pos = -1;
        while (1) {
            if (buffer_[bufptr] == ' ' || buffer_[bufptr] == '\0' || buffer_[bufptr] == '\n') { pos = bufptr; break; }
            bufptr++;
            if (bufptr >= sizeof(buffer_)) break;
        }
        if (pos == -1) {
            break;
        }
        endPos = pos;

        if (curIndex == index) {
            start = &buffer_[startPos];
            end   = &buffer_[endPos];
            break;
        } else {
            curIndex++;
        }
    }

    if (start != NULL && end != NULL) {
        // Copy to dst
        while (1) {
            *dst = *start;
            dst++;
            start++;
            if (start == end) {
                *dst = '\0';
                break;
            }
        }
    }

    return;
}
