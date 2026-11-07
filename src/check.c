#include "conv.h"

/*
 * Owner: Syed Ashhad Hassan.
 * Exact byte compare is the required small-image check.
 * Add the checksum-and-sample path for large images here.
 */

int check_image(const Image *got, const Image *ref, size_t *mismatch_count) {
    (void)got;
    (void)ref;
    (void)mismatch_count;
    convoscale_unimplemented("check_image", "Syed Ashhad Hassan");
    return -1;
}

int check_volume(const Volume *got, const Volume *ref, size_t *mismatch_count) {
    (void)got;
    (void)ref;
    (void)mismatch_count;
    convoscale_unimplemented("check_volume", "Syed Ashhad Hassan");
    return -1;
}
