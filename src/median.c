#include "conv.h"

/* Owner: Shaheer Khan. Median is not a weighted kernel. Windows 3, 5, 7, and 9. */

void median2d_seq(const Image *in, Image *out, int window) {
    (void)in;
    (void)out;
    (void)window;
    convoscale_unimplemented("median2d_seq", "Shaheer Khan");
}

void median2d_tiles(const Image *in, Image *out, int window, int threads, int tile) {
    (void)in;
    (void)out;
    (void)window;
    (void)threads;
    (void)tile;
    convoscale_unimplemented("median2d_tiles", "Shaheer Khan");
}
