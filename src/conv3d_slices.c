#include "conv.h"

/* Owner: Saad Shakeel. Slice-wise OpenMP, static schedule. */

void conv3d_slices(const Volume *in, Volume *out, const Kernel *kernel, int threads) {
    (void)in;
    (void)out;
    (void)kernel;
    (void)threads;
    convoscale_unimplemented("conv3d_slices", "Saad Shakeel");
}
