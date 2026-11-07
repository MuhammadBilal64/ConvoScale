#ifndef CONVOSCALE_FILTERS_H
#define CONVOSCALE_FILTERS_H

#include "conv.h"

/*
 * Linear filters build a Kernel and call conv2d_dispatch / conv3d_dispatch.
 * Sobel and Prewitt are two convolutions combined per pixel, so they do not
 * fit in one Kernel. Median stays in median2d_seq / median2d_tiles.
 *
 * Return 0 on success.
 */

/* Ashhad, src/filters_smooth.c */
int filter_gaussian2d(const Image *in, Image *out, int size, float sigma, const ConvParams *params);
int filter_box2d(const Image *in, Image *out, int size, const ConvParams *params);
int filter_gaussian3d(const Volume *in, Volume *out, int size, float sigma, const ConvParams *params);
int filter_mean3d(const Volume *in, Volume *out, int size, const ConvParams *params);

/* Saad, src/filters_sharpen.c */
int filter_sharpen2d(const Image *in, Image *out, const ConvParams *params);
int filter_emboss2d(const Image *in, Image *out, const ConvParams *params);

/* Bilal, src/filters_edge.c */
int filter_sobel2d(const Image *in, Image *out, const ConvParams *params);
int filter_prewitt2d(const Image *in, Image *out, const ConvParams *params);
int filter_laplacian2d(const Image *in, Image *out, const ConvParams *params);
int filter_sobel3d(const Volume *in, Volume *out, const ConvParams *params);

#endif
