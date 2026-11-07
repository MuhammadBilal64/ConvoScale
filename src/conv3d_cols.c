#include "conv.h"

/* Owner: Muhammad Bilal. Column-wise 3D OpenMP, parallel along width. */

void conv3d_cols(const Volume *in, Volume *out, const Kernel *kernel, int threads) {
    (void)in;
    (void)out;
    (void)kernel;
    (void)threads;
    convoscale_unimplemented("conv3d_cols", "Muhammad Bilal");
}
