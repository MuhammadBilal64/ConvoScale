#include "kernel.h"

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

static int weight_count(int ndim, int size, size_t *count) {
    int i;
    size_t n = 1;
    if ((ndim != 2 && ndim != 3) || size < 1 || size > 31 || (size % 2) == 0) {
        return -1;
    }
    for (i = 0; i < ndim; i++) {
        if (n > SIZE_MAX / (size_t)size) {
            return -1;
        }
        n *= (size_t)size;
    }
    *count = n;
    return 0;
}

Kernel *kernel_create(int ndim, int size) {
    size_t count;
    Kernel *kernel;
    if (weight_count(ndim, size, &count) != 0) {
        return NULL;
    }
    kernel = (Kernel *)malloc(sizeof(*kernel));
    if (kernel == NULL) {
        return NULL;
    }
    kernel->ndim = ndim;
    kernel->size = size;
    kernel->weights = (float *)calloc(count, sizeof(float));
    if (kernel->weights == NULL) {
        free(kernel);
        return NULL;
    }
    return kernel;
}

void kernel_destroy(Kernel *kernel) {
    if (kernel == NULL) {
        return;
    }
    free(kernel->weights);
    free(kernel);
}

int kernel_set_weights(Kernel *kernel, const float *weights, int count) {
    size_t expected;
    int i;
    if (kernel == NULL || weights == NULL || count < 0 ||
        weight_count(kernel->ndim, kernel->size, &expected) != 0 ||
        (size_t)count != expected) {
        return -1;
    }
    for (i = 0; i < count; i++) {
        kernel->weights[i] = weights[i];
    }
    return 0;
}

int kernel_fill_box(Kernel *kernel) {
    size_t count;
    size_t i;
    float weight;
    if (kernel == NULL || weight_count(kernel->ndim, kernel->size, &count) != 0 || count == 0) {
        return -1;
    }
    weight = 1.0f / (float)count;
    for (i = 0; i < count; i++) {
        kernel->weights[i] = weight;
    }
    return 0;
}

int kernel_fill_mean(Kernel *kernel) {
    return kernel_fill_box(kernel);
}
