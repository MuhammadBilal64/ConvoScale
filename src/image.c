#include "image.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int count_2d(int width, int height, size_t *count) {
    if (width <= 0 || height <= 0) {
        return -1;
    }
    if ((size_t)width > SIZE_MAX / (size_t)height) {
        return -1;
    }
    *count = (size_t)width * (size_t)height;
    return 0;
}

static int count_3d(int width, int height, int depth, size_t *count) {
    size_t plane;
    if (count_2d(width, height, &plane) != 0 || depth <= 0) {
        return -1;
    }
    if (plane > SIZE_MAX / (size_t)depth) {
        return -1;
    }
    *count = plane * (size_t)depth;
    return 0;
}

Image *image_create(int width, int height) {
    size_t count;
    Image *img;
    if (count_2d(width, height, &count) != 0) {
        return NULL;
    }
    img = (Image *)malloc(sizeof(*img));
    if (img == NULL) {
        return NULL;
    }
    img->width = width;
    img->height = height;
    img->data = (uint8_t *)calloc(count, 1);
    if (img->data == NULL) {
        free(img);
        return NULL;
    }
    return img;
}

void image_destroy(Image *img) {
    if (img == NULL) {
        return;
    }
    free(img->data);
    free(img);
}

void image_fill_pattern(Image *img) {
    int y;
    int x;
    if (img == NULL) {
        return;
    }
    for (y = 0; y < img->height; y++) {
        for (x = 0; x < img->width; x++) {
            image_set(img, x, y, (uint8_t)(((unsigned)x * 13u + (unsigned)y * 7u) & 255u));
        }
    }
}

int image_equal(const Image *a, const Image *b) {
    size_t count;
    if (a == NULL || b == NULL || a->width != b->width || a->height != b->height) {
        return 0;
    }
    if (count_2d(a->width, a->height, &count) != 0) {
        return 0;
    }
    return memcmp(a->data, b->data, count) == 0;
}

static int skip_ws_and_comments(FILE *fp) {
    int ch;
    for (;;) {
        ch = fgetc(fp);
        if (ch == '#') {
            while (ch != '\n' && ch != EOF) {
                ch = fgetc(fp);
            }
            continue;
        }
        if (ch == EOF) {
            return -1;
        }
        if (!isspace(ch)) {
            ungetc(ch, fp);
            return 0;
        }
    }
}

int image_save_pgm(const Image *img, const char *path) {
    FILE *fp;
    size_t count;
    if (img == NULL || path == NULL || count_2d(img->width, img->height, &count) != 0) {
        return -1;
    }
    fp = fopen(path, "wb");
    if (fp == NULL) {
        return -1;
    }
    if (fprintf(fp, "P5\n%d %d\n255\n", img->width, img->height) < 0 ||
        fwrite(img->data, 1, count, fp) != count) {
        fclose(fp);
        return -1;
    }
    if (fclose(fp) != 0) {
        return -1;
    }
    return 0;
}

Image *image_load_pgm(const char *path) {
    FILE *fp;
    char magic[3];
    int width;
    int height;
    int maxval;
    size_t count;
    Image *img;
    if (path == NULL) {
        return NULL;
    }
    fp = fopen(path, "rb");
    if (fp == NULL) {
        return NULL;
    }
    if (fgets(magic, (int)sizeof(magic), fp) == NULL || strcmp(magic, "P5") != 0) {
        fclose(fp);
        return NULL;
    }
    if (skip_ws_and_comments(fp) != 0 || fscanf(fp, "%d", &width) != 1 ||
        skip_ws_and_comments(fp) != 0 || fscanf(fp, "%d", &height) != 1 ||
        skip_ws_and_comments(fp) != 0 || fscanf(fp, "%d", &maxval) != 1) {
        fclose(fp);
        return NULL;
    }
    if (maxval != 255 || fgetc(fp) == EOF) {
        fclose(fp);
        return NULL;
    }
    img = image_create(width, height);
    if (img == NULL) {
        fclose(fp);
        return NULL;
    }
    count = (size_t)width * (size_t)height;
    if (fread(img->data, 1, count, fp) != count) {
        image_destroy(img);
        fclose(fp);
        return NULL;
    }
    fclose(fp);
    return img;
}

Volume *volume_create(int width, int height, int depth) {
    size_t count;
    Volume *vol;
    if (count_3d(width, height, depth, &count) != 0) {
        return NULL;
    }
    vol = (Volume *)malloc(sizeof(*vol));
    if (vol == NULL) {
        return NULL;
    }
    vol->width = width;
    vol->height = height;
    vol->depth = depth;
    vol->data = (uint8_t *)calloc(count, 1);
    if (vol->data == NULL) {
        free(vol);
        return NULL;
    }
    return vol;
}

void volume_destroy(Volume *vol) {
    if (vol == NULL) {
        return;
    }
    free(vol->data);
    free(vol);
}

void volume_fill_pattern(Volume *vol) {
    int z;
    int y;
    int x;
    if (vol == NULL) {
        return;
    }
    for (z = 0; z < vol->depth; z++) {
        for (y = 0; y < vol->height; y++) {
            for (x = 0; x < vol->width; x++) {
                vol->data[volume_index(vol, x, y, z)] =
                    (uint8_t)(((unsigned)x * 13u + (unsigned)y * 7u + (unsigned)z * 3u) & 255u);
            }
        }
    }
}

int volume_equal(const Volume *a, const Volume *b) {
    size_t count;
    if (a == NULL || b == NULL || a->width != b->width || a->height != b->height || a->depth != b->depth) {
        return 0;
    }
    if (count_3d(a->width, a->height, a->depth, &count) != 0) {
        return 0;
    }
    return memcmp(a->data, b->data, count) == 0;
}

int volume_save(const Volume *vol, const char *path) {
    FILE *fp;
    size_t count;
    if (vol == NULL || path == NULL || count_3d(vol->width, vol->height, vol->depth, &count) != 0) {
        return -1;
    }
    fp = fopen(path, "wb");
    if (fp == NULL) {
        return -1;
    }
    if (fprintf(fp, "CVOL\n%d %d %d\n", vol->width, vol->height, vol->depth) < 0 ||
        fwrite(vol->data, 1, count, fp) != count) {
        fclose(fp);
        return -1;
    }
    if (fclose(fp) != 0) {
        return -1;
    }
    return 0;
}

Volume *volume_load(const char *path) {
    FILE *fp;
    char magic[8];
    int width;
    int height;
    int depth;
    size_t count;
    Volume *vol;
    if (path == NULL) {
        return NULL;
    }
    fp = fopen(path, "rb");
    if (fp == NULL) {
        return NULL;
    }
    if (fgets(magic, (int)sizeof(magic), fp) == NULL || strcmp(magic, "CVOL\n") != 0) {
        fclose(fp);
        return NULL;
    }
    if (fscanf(fp, "%d %d %d", &width, &height, &depth) != 3 || fgetc(fp) == EOF) {
        fclose(fp);
        return NULL;
    }
    vol = volume_create(width, height, depth);
    if (vol == NULL) {
        fclose(fp);
        return NULL;
    }
    count = (size_t)width * (size_t)height * (size_t)depth;
    if (fread(vol->data, 1, count, fp) != count) {
        volume_destroy(vol);
        fclose(fp);
        return NULL;
    }
    fclose(fp);
    return vol;
}
