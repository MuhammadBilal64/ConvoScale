#include "filters.h"

/*
 * Owner: Saad Shakeel.
 * Usual 3x3 kernels, row-major:
 *   sharpen  0 -1 0 / -1 5 -1 / 0 -1 0
 *   emboss  -2 -1 0 / -1 1 1 / 0 1 2
 */

int filter_sharpen2d(const Image *in, Image *out, const ConvParams *params) {
    (void)in;
    (void)out;
    (void)params;
    convoscale_unimplemented("filter_sharpen2d", "Saad Shakeel");
    return -1;
}

int filter_emboss2d(const Image *in, Image *out, const ConvParams *params) {
    (void)in;
    (void)out;
    (void)params;
    convoscale_unimplemented("filter_emboss2d", "Saad Shakeel");
    return -1;
}
