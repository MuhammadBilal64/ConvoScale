#include "conv.h"

/* Owner: Shaheer Khan. Tile-wise 3D OpenMP. */

void conv3d_tiles(const Volume *in, Volume *out, const Kernel *kernel, int threads, int tile) {
    (void)in;
    (void)out;
    (void)kernel;
    (void)threads;
    (void)tile;
    convoscale_unimplemented("conv3d_tiles", "Shaheer Khan");
}
