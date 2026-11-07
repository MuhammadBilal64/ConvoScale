#ifndef CONVOSCALE_CONV_H
#define CONVOSCALE_CONV_H

#include "image.h"
#include "kernel.h"

typedef enum Schedule {
    SCHED_STATIC = 0,
    SCHED_DYNAMIC,
    SCHED_GUIDED
} Schedule;

typedef enum ConvMode {
    CONV_SEQ = 0,
    CONV_ROWS,
    CONV_COLS,
    CONV_TILES
} ConvMode;

typedef struct ConvParams {
    ConvMode mode;
    int threads; /* >= 1 after conv*_dispatch */
    Schedule schedule;
    int chunk; /* 0 selects the OpenMP default chunk */
    int tile;
} ConvParams;

/*
 * Convert a float accumulator to the stored pixel.
 * Nearest uint8, clamped to 0..255. Every version uses this.
 */
static inline uint8_t conv_to_u8(float value) {
    if (!(value > 0.0f)) {
        return 0;
    }
    if (value >= 255.0f) {
        return 255;
    }
    return (uint8_t)(value + 0.5f);
}

/* Set by stubs and by convoscale_fail. Real code leaves this at 0. */
void convoscale_status_reset(void);
int convoscale_status(void);
void convoscale_fail(const char *message);
void convoscale_unimplemented(const char *function_name, const char *owner);

/* Bilal, src/dispatch.c. Validates sizes, then calls the mode below. */
int conv2d_dispatch(const Image *in, Image *out, const Kernel *kernel, const ConvParams *params);
int conv3d_dispatch(const Volume *in, Volume *out, const Kernel *kernel, const ConvParams *params);

/* Ashhad, src/conv2d_seq.c and src/conv3d_seq.c */
void conv2d_seq(const Image *in, Image *out, const Kernel *kernel);
void conv3d_seq(const Volume *in, Volume *out, const Kernel *kernel);

/* Saad, src/conv2d_rows.c and src/conv3d_slices.c. Slices are static. */
void conv2d_rows(const Image *in, Image *out, const Kernel *kernel,
                 int threads, Schedule schedule, int chunk);
void conv3d_slices(const Volume *in, Volume *out, const Kernel *kernel, int threads);

/* Bilal, src/conv2d_cols.c and src/conv3d_cols.c */
void conv2d_cols(const Image *in, Image *out, const Kernel *kernel, int threads);
void conv3d_cols(const Volume *in, Volume *out, const Kernel *kernel, int threads);

/* Shaheer, src/conv2d_tiles.c and src/conv3d_tiles.c */
void conv2d_tiles(const Image *in, Image *out, const Kernel *kernel, int threads, int tile);
void conv3d_tiles(const Volume *in, Volume *out, const Kernel *kernel, int threads, int tile);

/* Shaheer, src/median.c. Median is not a Kernel. */
void median2d_seq(const Image *in, Image *out, int window);
void median2d_tiles(const Image *in, Image *out, int window, int threads, int tile);

/* Ashhad, src/generate.c */
int generate_benchmark_image(Image *img);
int generate_benchmark_volume(Volume *vol);

/* Ashhad, src/check.c. mismatch_count may be NULL. Returns 0 when equal. */
int check_image(const Image *got, const Image *ref, size_t *mismatch_count);
int check_volume(const Volume *got, const Volume *ref, size_t *mismatch_count);

/* Saad, src/batch.c */
int batch_run(const char *directory, int threads,
              double *sequential_seconds, double *parallel_seconds);

int schedule_parse(const char *text, Schedule *out);
const char *schedule_name(Schedule schedule);
int conv_mode_parse(const char *text, ConvMode *out);
const char *conv_mode_name(ConvMode mode);

#endif
