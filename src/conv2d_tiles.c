#include "conv.h"

/* Owner: Shaheer Khan. Tile-wise 2D OpenMP with collapse(2). */

void conv2d_tiles(const Image *in, Image *out, const Kernel *kernel, int threads, int tile) {
    (void)in;
    (void)out;
    (void)kernel;
    (void)threads;
    (void)tile;
    convoscale_unimplemented("conv2d_tiles", "Shaheer Khan");
}
