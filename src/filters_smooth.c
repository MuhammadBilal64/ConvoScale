#include "filters.h"

/* Owner: Syed Ashhad Hassan. Gaussian and box / mean. Keep the signatures. */

int filter_gaussian2d(const Image *in, Image *out, int size, float sigma, const ConvParams *params) {
    (void)in;
    (void)out;
    (void)size;
    (void)sigma;
    (void)params;
    convoscale_unimplemented("filter_gaussian2d", "Syed Ashhad Hassan");
    return -1;
}

int filter_box2d(const Image *in, Image *out, int size, const ConvParams *params) {
    (void)in;
    (void)out;
    (void)size;
    (void)params;
    convoscale_unimplemented("filter_box2d", "Syed Ashhad Hassan");
    return -1;
}

int filter_gaussian3d(const Volume *in, Volume *out, int size, float sigma, const ConvParams *params) {
    (void)in;
    (void)out;
    (void)size;
    (void)sigma;
    (void)params;
    convoscale_unimplemented("filter_gaussian3d", "Syed Ashhad Hassan");
    return -1;
}

int filter_mean3d(const Volume *in, Volume *out, int size, const ConvParams *params) {
    (void)in;
    (void)out;
    (void)size;
    (void)params;
    convoscale_unimplemented("filter_mean3d", "Syed Ashhad Hassan");
    return -1;
}
