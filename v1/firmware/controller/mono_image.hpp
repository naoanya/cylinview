/**********************************************************************/
/**
 * @brief  Monochrome (8bit Packed) Image
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
 * Class definitions
 *----------------------------------------------------------------------
 */

typedef struct mono_image_ {
    int width_;
    int height_;
    int draw_offset_x_;
    int draw_offset_y_;
    int data_width_;        // Same as width_
    int data_height_;       // Height include 8 bit packing. e.g. height_ is 13, then data_height_ is 16
    int data_size_;
    bool has_alpha_;        // Has alpha flag. true : alphabuffer_ is valid pointer, false : alphabuffer_ is NULL.
    const uint8_t * buffer_;      // Data buffer pointer.
    const uint8_t * alphabuffer_; // Alpha data buffer. same size to buffer_, 1 = opaque, 0 = transparent
} mono_image_t;

typedef struct mono_images_ {
    int count_;
    const mono_image_t * images_;
} mono_images_t;

class MonoImage
{
public:
    explicit MonoImage(const mono_image_t * image) {
        image_ = image;
    }
    virtual ~MonoImage() {}

public:
    int width(void) { return image_->width_; }
    int height(void) { return image_->height_; }
    int pixels(void) { return image_->width_ * image_->height_; };

public:
    int drawOffsetX(void) { return image_->draw_offset_x_; }
    int drawOffsetY(void) { return image_->draw_offset_y_; }
    bool hasAlpha(void) { return image_->has_alpha_; }

public:
    uint8_t getDot(int x, int y) {
        int offset_stride = (y / 8) * width();
        int offset_bit    = y % 8;
        return (image_->buffer_[x + offset_stride] >> offset_bit) & 0x01;
    }
    uint8_t getDotAlpha(int x, int y) {
        int offset_stride = (y / 8) * width();
        int offset_bit    = y % 8;
        return (image_->alphabuffer_[x + offset_stride] >> offset_bit) & 0x01;
    }

public:
    int buffferHeight(void) { return ((height() + 7) / 8) * 8; }
    int buffferSize(void) { return (width() * buffferHeight()) / 8; }
    const uint8_t * getBuffer(void) { return image_->buffer_; }

public:
    const mono_image_t * image_;
};

