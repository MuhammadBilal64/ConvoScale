#ifndef CONVOSCALE_IMAGE_H
#define CONVOSCALE_IMAGE_H

#include <stddef.h>
#include <stdint.h>

/* Row-major uint8 image. data length is width * height. */
typedef struct Image {
    int width;
    int height;
    uint8_t *data;
} Image;

/*
 * Row-major volume. Index is x + width * (y + height * z):
 * x is contiguous, then y, then z.
 */
typedef struct Volume {
    int width;
    int height;
    int depth;
    uint8_t *data;
} Volume;

Image *image_create(int width, int height);
void image_destroy(Image *img);
void image_fill_pattern(Image *img);

/* In bounds only. Hot loops use this. */
static inline void image_set(Image *img, int x, int y, uint8_t value) {
    img->data[(size_t)y * (size_t)img->width + (size_t)x] = value;
}

/* Zero outside the image. Negative coordinates count as outside. */
static inline uint8_t image_pixel(const Image *img, int x, int y) {
    if ((unsigned)x >= (unsigned)img->width || (unsigned)y >= (unsigned)img->height) {
        return 0;
    }
    return img->data[(size_t)y * (size_t)img->width + (size_t)x];
}

int image_save_pgm(const Image *img, const char *path);
Image *image_load_pgm(const char *path);
int image_equal(const Image *a, const Image *b);

Volume *volume_create(int width, int height, int depth);
void volume_destroy(Volume *vol);
void volume_fill_pattern(Volume *vol);

static inline size_t volume_index(const Volume *vol, int x, int y, int z) {
    return (size_t)x + (size_t)vol->width * ((size_t)y + (size_t)vol->height * (size_t)z);
}

static inline uint8_t volume_pixel(const Volume *vol, int x, int y, int z) {
    if ((unsigned)x >= (unsigned)vol->width ||
        (unsigned)y >= (unsigned)vol->height ||
        (unsigned)z >= (unsigned)vol->depth) {
        return 0;
    }
    return vol->data[volume_index(vol, x, y, z)];
}

int volume_save(const Volume *vol, const char *path);
Volume *volume_load(const char *path);
int volume_equal(const Volume *a, const Volume *b);

#endif
