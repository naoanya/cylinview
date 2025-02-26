/**********************************************************************/
/**
 * @brief  Circular Buffer
 * @author naoa
 */
/**********************************************************************/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Include files
 *----------------------------------------------------------------------
 */
#include <cstdio>
#include <cstring>

#include "circular_buffer.hpp"

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Function definitions
 *----------------------------------------------------------------------
 */

CircularBuffer::CircularBuffer() :
    bufPtr_(nullptr),
    wrBufPtr_(nullptr),
    rdBufPtr_(nullptr),
    size_(0),
    offset_(0),
    num_(0)
{
    reset();
}

CircularBuffer::~CircularBuffer()
{
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Function definitions
 *----------------------------------------------------------------------
 */

void
CircularBuffer::setBuffer(uint8_t * buffer, size_t size, int num)
{
    if (num > CIRCULAR_BUFFER_MAX_NUM) {
        printf("WARN: Invalid supported buffer num.\n");
        num = CIRCULAR_BUFFER_MAX_NUM;
    }
    if ((size_ % num_) != 0) {
        printf("WARN: The num is not an integer multiple of the buffer size\n");
    }

    bufPtr_ = buffer;
    size_ = size;
    num_ = num;
    offset_ = (size_ / num_);
    endBufPtr_ = bufPtr_ + size_;
    reset();
}

void
CircularBuffer::reset(void)
{
    wr_ = 0;
    rd_ = 0;
    memset((void*)full_, 0, sizeof(full_));
    wrBufPtr_ = bufPtr_;
    rdBufPtr_ = bufPtr_;
}

bool
CircularBuffer::getWriteReady(void)
{
    return !full_[wr_];
}

bool
CircularBuffer::getReadReady(void)
{
    return full_[rd_];
}

uint8_t *
CircularBuffer::getWriteBufferPtr(void)
{
    return wrBufPtr_;
}

uint8_t *
CircularBuffer::getReadBufferPtr(void)
{
    return rdBufPtr_;
}

void
CircularBuffer::nextWriteBuffer(void)
{
    full_[wr_] = true;
    wr_++;
    if (wr_ >= num_) wr_ = 0;
    wrBufPtr_ += offset_;
    if (wrBufPtr_ >= endBufPtr_) wrBufPtr_ = bufPtr_;
}

void
CircularBuffer::nextReadBuffer(void)
{
    full_[rd_] = false;
    rd_++;
    if (rd_ >= num_) rd_ = 0;
    rdBufPtr_ += offset_;
    if (rdBufPtr_ >= endBufPtr_) rdBufPtr_ = bufPtr_;
}

