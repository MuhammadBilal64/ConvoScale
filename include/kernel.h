#ifndef CONVOSCALE_KERNEL_H
#define CONVOSCALE_KERNEL_H

/* ndim is 2 or 3. size is the odd edge length. weights length is size^ndim. */
typedef struct Kernel {
    int ndim;
    int size;
    float *weights;
} Kernel;

/* Bilal, src/kernel_util.c. size must be odd and in 1..31. */
Kernel *kernel_create(int ndim, int size);
void kernel_destroy(Kernel *kernel);
int kernel_set_weights(Kernel *kernel, const float *weights, int count);
int kernel_fill_box(Kernel *kernel);
int kernel_fill_mean(Kernel *kernel);

/*
 * Ashhad, src/kernel.c.
 * Gaussian weights for the kernel's ndim and size. sigma > 0.
 * Returns 0 on success.
 */
int kernel_fill_gaussian(Kernel *kernel, float sigma);

#endif
