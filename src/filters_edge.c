#include "filters.h"

/*
 * Owner: Muhammad Bilal.
 * Sobel and Prewitt each need two convolutions (x and y) combined per pixel.
 * Laplacian is one kernel, commonly:
 *   0  1 0 / 1 -4 1 / 0 1 0
 */

int filter_sobel2d(const Image *in, Image *out, const ConvParams *params) {
    (void)in;
    (void)out;
    (void)params;
    convoscale_unimplemented("filter_sobel2d", "Muhammad Bilal");
    return -1;
}

int filter_prewitt2d(const Image *in, Image *out, const ConvParams *params) {
    (void)in;
    (void)out;
    (void)params;
    convoscale_unimplemented("filter_prewitt2d", "Muhammad Bilal");
    return -1;
}

int filter_laplacian2d(const Image *in, Image *out, const ConvParams *params) {
    (void)in;
    (void)out;
    (void)params;
    convoscale_unimplemented("filter_laplacian2d", "Muhammad Bilal");
    return -1;
}

int filter_sobel3d(const Volume *in, Volume *out, const ConvParams *params) {
    (void)in;
    (void)out;
    (void)params;
    convoscale_unimplemented("filter_sobel3d", "Muhammad Bilal");
    return -1;
}
