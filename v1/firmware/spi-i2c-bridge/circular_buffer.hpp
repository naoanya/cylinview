/**********************************************************************/
/**
 * @brief  Circular Buffer
 * @author naoa
 */
/**********************************************************************/
#pragma once
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Include files
 *----------------------------------------------------------------------
 */
#include <cstdint>
#include <cstdbool>

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Definitions - Config
 *----------------------------------------------------------------------
 */

#define CIRCULAR_BUFFER_MAX_NUM (2)

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Class definitions
 *----------------------------------------------------------------------
 */

class CircularBuffer
{
public:
    explicit CircularBuffer();
    virtual ~CircularBuffer();

public:
    void setBuffer(uint8_t * buffer, size_t size, int num);
    void reset(void);

    bool        getWriteReady(void);
    bool        getReadReady(void);
    uint8_t *   getWriteBufferPtr(void);
    uint8_t *   getReadBufferPtr(void);
    void        nextWriteBuffer(void);
    void        nextReadBuffer(void);

private:
    uint8_t * bufPtr_;
    size_t size_;
    int num_;
    size_t offset_;
    uint8_t * endBufPtr_;
    bool full_[CIRCULAR_BUFFER_MAX_NUM];
    int wr_;
    int rd_;
    uint8_t * wrBufPtr_;
    uint8_t * rdBufPtr_;
};
