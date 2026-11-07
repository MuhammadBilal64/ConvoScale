#include "conv.h"

/* Owner: Saad Shakeel. Row-wise OpenMP, including static, dynamic, and guided. */

void conv2d_rows(const Image *in, Image *out, const Kernel *kernel,
                 int threads, Schedule schedule, int chunk) {
    (void)in;
    (void)out;
    (void)kernel;
    (void)threads;
    (void)schedule;
    (void)chunk;
    convoscale_unimplemented("conv2d_rows", "Saad Shakeel");
}
