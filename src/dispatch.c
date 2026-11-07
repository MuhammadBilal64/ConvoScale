#include "conv.h"

#include <stdio.h>
#include <string.h>

static int g_status = 0;

void convoscale_status_reset(void) {
    g_status = 0;
}

int convoscale_status(void) {
    return g_status;
}

void convoscale_fail(const char *message) {
    if (message != NULL) {
        fprintf(stderr, "%s\n", message);
    }
    g_status = 1;
}

void convoscale_unimplemented(const char *function_name, const char *owner) {
    fprintf(stderr, "%s: not implemented (%s)\n",
            function_name != NULL ? function_name : "function",
            owner != NULL ? owner : "unassigned");
    g_status = 1;
}

int schedule_parse(const char *text, Schedule *out) {
    if (text == NULL || out == NULL) {
        return -1;
    }
    if (strcmp(text, "static") == 0) {
        *out = SCHED_STATIC;
        return 0;
    }
    if (strcmp(text, "dynamic") == 0) {
        *out = SCHED_DYNAMIC;
        return 0;
    }
    if (strcmp(text, "guided") == 0) {
        *out = SCHED_GUIDED;
        return 0;
    }
    return -1;
}

const char *schedule_name(Schedule schedule) {
    switch (schedule) {
    case SCHED_STATIC:
        return "static";
    case SCHED_DYNAMIC:
        return "dynamic";
    case SCHED_GUIDED:
        return "guided";
    default:
        return "na";
    }
}

int conv_mode_parse(const char *text, ConvMode *out) {
    if (text == NULL || out == NULL) {
        return -1;
    }
    if (strcmp(text, "seq") == 0) {
        *out = CONV_SEQ;
        return 0;
    }
    if (strcmp(text, "rows") == 0) {
        *out = CONV_ROWS;
        return 0;
    }
    if (strcmp(text, "cols") == 0) {
        *out = CONV_COLS;
        return 0;
    }
    if (strcmp(text, "tiles") == 0) {
        *out = CONV_TILES;
        return 0;
    }
    return -1;
}

const char *conv_mode_name(ConvMode mode) {
    switch (mode) {
    case CONV_SEQ:
        return "seq";
    case CONV_ROWS:
        return "rows";
    case CONV_COLS:
        return "cols";
    case CONV_TILES:
        return "tiles";
    default:
        return "na";
    }
}

static int prepare_params(ConvParams *params) {
    if (params == NULL) {
        convoscale_fail("missing convolution parameters");
        return -1;
    }
    if (params->threads < 1) {
        params->threads = 1;
    }
    if (params->tile < 1) {
        convoscale_fail("tile size must be >= 1");
        return -1;
    }
    return 0;
}

int conv2d_dispatch(const Image *in, Image *out, const Kernel *kernel, const ConvParams *params) {
    ConvParams local;
    convoscale_status_reset();
    if (in == NULL || out == NULL || kernel == NULL || in->data == NULL || out->data == NULL) {
        convoscale_fail("conv2d: null image or kernel");
        return -1;
    }
    if (in->width != out->width || in->height != out->height) {
        convoscale_fail("conv2d: input and output sizes differ");
        return -1;
    }
    if (kernel->ndim != 2 || kernel->weights == NULL) {
        convoscale_fail("conv2d: kernel must be 2D");
        return -1;
    }
    local = *params;
    if (prepare_params(&local) != 0) {
        return -1;
    }
    switch (local.mode) {
    case CONV_SEQ:
        conv2d_seq(in, out, kernel);
        break;
    case CONV_ROWS:
        conv2d_rows(in, out, kernel, local.threads, local.schedule, local.chunk);
        break;
    case CONV_COLS:
        conv2d_cols(in, out, kernel, local.threads);
        break;
    case CONV_TILES:
        conv2d_tiles(in, out, kernel, local.threads, local.tile);
        break;
    default:
        convoscale_fail("conv2d: unknown mode");
        break;
    }
    return convoscale_status() == 0 ? 0 : -1;
}

int conv3d_dispatch(const Volume *in, Volume *out, const Kernel *kernel, const ConvParams *params) {
    ConvParams local;
    convoscale_status_reset();
    if (in == NULL || out == NULL || kernel == NULL || in->data == NULL || out->data == NULL) {
        convoscale_fail("conv3d: null volume or kernel");
        return -1;
    }
    if (in->width != out->width || in->height != out->height || in->depth != out->depth) {
        convoscale_fail("conv3d: input and output sizes differ");
        return -1;
    }
    if (kernel->ndim != 3 || kernel->weights == NULL) {
        convoscale_fail("conv3d: kernel must be 3D");
        return -1;
    }
    local = *params;
    if (prepare_params(&local) != 0) {
        return -1;
    }
    switch (local.mode) {
    case CONV_SEQ:
        conv3d_seq(in, out, kernel);
        break;
    case CONV_ROWS:
        conv3d_slices(in, out, kernel, local.threads);
        break;
    case CONV_COLS:
        conv3d_cols(in, out, kernel, local.threads);
        break;
    case CONV_TILES:
        conv3d_tiles(in, out, kernel, local.threads, local.tile);
        break;
    default:
        convoscale_fail("conv3d: unknown mode");
        break;
    }
    return convoscale_status() == 0 ? 0 : -1;
}
