#include "conv.h"

/* Owner: Saad Shakeel. Directory of real images, sequential vs one image per thread. */

int batch_run(const char *directory, int threads,
              double *sequential_seconds, double *parallel_seconds) {
    (void)directory;
    (void)threads;
    (void)sequential_seconds;
    (void)parallel_seconds;
    convoscale_unimplemented("batch_run", "Saad Shakeel");
    return -1;
}
