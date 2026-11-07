#include "convoscale.h"

#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Options {
    const char *mode_text;
    const char *in_path;
    const char *out_path;
    const char *path_a;
    const char *path_b;
    const char *directory;
    int width;
    int height;
    int depth;
    int size;
    int threads;
    int chunk;
    int tile;
    int window;
    float sigma;
    Schedule schedule;
} Options;

static void options_init(Options *opt) {
    memset(opt, 0, sizeof(*opt));
    opt->width = -1;
    opt->height = -1;
    opt->depth = -1;
    opt->size = -1;
    opt->window = -1;
    opt->tile = 32;
    opt->sigma = 1.0f;
    opt->schedule = SCHED_STATIC;
}

static void usage(void) {
    fprintf(stderr,
            "Usage:\n"
            "  convoscale info\n"
            "  convoscale self-test\n"
            "  convoscale generate2d --width W --height H --out FILE.pgm\n"
            "  convoscale generate3d --width W --height H --depth D --out FILE.cvol\n"
            "  convoscale conv2d --mode seq|rows|cols|tiles --in IN.pgm --out OUT.pgm --size N\n"
            "            [--threads N] [--schedule static|dynamic|guided] [--chunk N] [--tile N]\n"
            "  convoscale conv3d --mode seq|rows|cols|tiles --in IN.cvol --out OUT.cvol --size N\n"
            "            [--threads N] [--tile N]\n"
            "  convoscale median2d --mode seq|tiles --in IN.pgm --out OUT.pgm --window N\n"
            "            [--threads N] [--tile N]\n"
            "  convoscale filter NAME [same flags as conv2d/conv3d]\n"
            "            names: gaussian2d box2d gaussian3d mean3d sharpen2d emboss2d\n"
            "                   sobel2d prewitt2d laplacian2d sobel3d\n"
            "  convoscale cmp --a A.pgm --b B.pgm\n"
            "  convoscale cmp3d --a A.cvol --b B.cvol\n"
            "  convoscale check --a GOT --b REF\n"
            "  convoscale benchgen --width W --height H [--depth D] --out FILE\n"
            "  convoscale batch --dir DIR [--threads N]\n"
            "\n"
            "generate2d/3d write a deterministic test pattern. Benchmark generation is benchgen.\n"
            "conv2d/conv3d pass a uniform box kernel. Filter names call each owner's function.\n"
            "Defaults: threads = core count, schedule = static, chunk = 0, tile = 32, sigma = 1.\n");
}

static int parse_options(int argc, char **argv, int start, Options *opt) {
    int i;
    for (i = start; i < argc; i++) {
        const char *flag = argv[i];
        if (i + 1 >= argc) {
            fprintf(stderr, "missing value for %s\n", flag);
            return -1;
        }
        if (strcmp(flag, "--mode") == 0) {
            opt->mode_text = argv[++i];
        } else if (strcmp(flag, "--in") == 0) {
            opt->in_path = argv[++i];
        } else if (strcmp(flag, "--out") == 0) {
            opt->out_path = argv[++i];
        } else if (strcmp(flag, "--a") == 0) {
            opt->path_a = argv[++i];
        } else if (strcmp(flag, "--b") == 0) {
            opt->path_b = argv[++i];
        } else if (strcmp(flag, "--dir") == 0) {
            opt->directory = argv[++i];
        } else if (strcmp(flag, "--width") == 0) {
            opt->width = atoi(argv[++i]);
        } else if (strcmp(flag, "--height") == 0) {
            opt->height = atoi(argv[++i]);
        } else if (strcmp(flag, "--depth") == 0) {
            opt->depth = atoi(argv[++i]);
        } else if (strcmp(flag, "--size") == 0) {
            opt->size = atoi(argv[++i]);
        } else if (strcmp(flag, "--threads") == 0) {
            opt->threads = atoi(argv[++i]);
        } else if (strcmp(flag, "--chunk") == 0) {
            opt->chunk = atoi(argv[++i]);
        } else if (strcmp(flag, "--tile") == 0) {
            opt->tile = atoi(argv[++i]);
        } else if (strcmp(flag, "--window") == 0) {
            opt->window = atoi(argv[++i]);
        } else if (strcmp(flag, "--sigma") == 0) {
            opt->sigma = (float)atof(argv[++i]);
        } else if (strcmp(flag, "--schedule") == 0) {
            if (schedule_parse(argv[++i], &opt->schedule) != 0) {
                fprintf(stderr, "schedule must be static, dynamic, or guided\n");
                return -1;
            }
        } else {
            fprintf(stderr, "unknown argument: %s\n", flag);
            return -1;
        }
    }
    return 0;
}

static int params_from_options(const Options *opt, ConvParams *params) {
    if (opt->mode_text == NULL || conv_mode_parse(opt->mode_text, &params->mode) != 0) {
        fprintf(stderr, "need --mode seq|rows|cols|tiles\n");
        return -1;
    }
    params->threads = opt->threads > 0 ? opt->threads : omp_get_max_threads();
    if (params->threads < 1) {
        params->threads = 1;
    }
    params->schedule = opt->schedule;
    params->chunk = opt->chunk;
    params->tile = opt->tile;
    return 0;
}

static int cmd_info(void) {
    printf("ConvoScale\n");
    printf("OpenMP max threads: %d\n", omp_get_max_threads());
    printf("pixels: row-major uint8, zero padding, output rounded with conv_to_u8\n");
    printf("modes: seq, rows, cols, tiles\n");
    return 0;
}

static int expect(int cond, const char *label) {
    if (cond) {
        printf("  ok  %s\n", label);
        return 0;
    }
    printf("  FAIL %s\n", label);
    return 1;
}

static int cmd_self_test(void) {
    int failed = 0;
    Image *img;
    Image *loaded;
    Image *out;
    Volume *vol;
    Volume *vol_loaded;
    Kernel *kernel;
    Kernel *rejected;
    ConvParams params;
    float sum;
    int i;

    img = image_create(6, 4);
    image_fill_pattern(img);
    failed += expect(img != NULL && image_pixel(img, 0, 0) == 0, "origin pixel");
    failed += expect(image_pixel(img, 1, 0) == 13, "pattern pixel");
    failed += expect(image_pixel(img, -1, 0) == 0 && image_pixel(img, 6, 0) == 0, "zero padding");
    failed += expect(image_save_pgm(img, "data/_selftest.pgm") == 0, "save pgm");
    loaded = image_load_pgm("data/_selftest.pgm");
    failed += expect(image_equal(img, loaded), "pgm roundtrip");

    vol = volume_create(3, 2, 2);
    volume_fill_pattern(vol);
    failed += expect(volume_pixel(vol, 1, 0, 1) == (uint8_t)((13u + 3u) & 255u), "volume pixel");
    failed += expect(volume_pixel(vol, -1, 0, 0) == 0, "volume zero padding");
    failed += expect(volume_save(vol, "data/_selftest.cvol") == 0, "save cvol");
    vol_loaded = volume_load("data/_selftest.cvol");
    failed += expect(volume_equal(vol, vol_loaded), "cvol roundtrip");

    rejected = kernel_create(2, 4);
    kernel = kernel_create(2, 3);
    failed += expect(rejected == NULL && kernel != NULL, "odd kernel size");
    failed += expect(kernel_fill_box(kernel) == 0, "box kernel");
    sum = 0.0f;
    for (i = 0; i < 9; i++) {
        sum += kernel->weights[i];
    }
    failed += expect(sum > 0.999f && sum < 1.001f, "box weights sum to 1");
    failed += expect(conv_to_u8(-1.0f) == 0 && conv_to_u8(255.4f) == 255 && conv_to_u8(1.5f) == 2,
                     "rounding");

    out = image_create(img->width, img->height);
    params.mode = CONV_SEQ;
    params.threads = 1;
    params.schedule = SCHED_STATIC;
    params.chunk = 0;
    params.tile = 32;
    failed += expect(conv2d_dispatch(img, out, kernel, &params) != 0, "sequential stub is linked");

    image_destroy(img);
    image_destroy(loaded);
    image_destroy(out);
    volume_destroy(vol);
    volume_destroy(vol_loaded);
    kernel_destroy(kernel);
    remove("data/_selftest.pgm");
    remove("data/_selftest.cvol");

    if (failed != 0) {
        fprintf(stderr, "self-test failed (%d)\n", failed);
        return 1;
    }
    printf("self-test passed\n");
    return 0;
}

static int cmd_generate2d(const Options *opt) {
    Image *img;
    int rc;
    if (opt->width < 1 || opt->height < 1 || opt->out_path == NULL) {
        fprintf(stderr, "generate2d needs --width, --height, and --out\n");
        return 1;
    }
    img = image_create(opt->width, opt->height);
    if (img == NULL) {
        fprintf(stderr, "could not allocate image\n");
        return 1;
    }
    image_fill_pattern(img);
    rc = image_save_pgm(img, opt->out_path);
    image_destroy(img);
    if (rc != 0) {
        fprintf(stderr, "could not write %s\n", opt->out_path);
        return 1;
    }
    return 0;
}

static int cmd_generate3d(const Options *opt) {
    Volume *vol;
    int rc;
    if (opt->width < 1 || opt->height < 1 || opt->depth < 1 || opt->out_path == NULL) {
        fprintf(stderr, "generate3d needs --width, --height, --depth, and --out\n");
        return 1;
    }
    vol = volume_create(opt->width, opt->height, opt->depth);
    if (vol == NULL) {
        fprintf(stderr, "could not allocate volume\n");
        return 1;
    }
    volume_fill_pattern(vol);
    rc = volume_save(vol, opt->out_path);
    volume_destroy(vol);
    if (rc != 0) {
        fprintf(stderr, "could not write %s\n", opt->out_path);
        return 1;
    }
    return 0;
}

static int cmd_conv2d(const Options *opt) {
    ConvParams params;
    Image *in;
    Image *out;
    Kernel *kernel;
    int rc;
    if (params_from_options(opt, &params) != 0 || opt->in_path == NULL || opt->out_path == NULL ||
        opt->size < 1) {
        fprintf(stderr, "conv2d needs --mode, --in, --out, and --size\n");
        return 1;
    }
    in = image_load_pgm(opt->in_path);
    if (in == NULL) {
        fprintf(stderr, "could not read %s\n", opt->in_path);
        return 1;
    }
    out = image_create(in->width, in->height);
    kernel = kernel_create(2, opt->size);
    if (out == NULL || kernel == NULL || kernel_fill_box(kernel) != 0) {
        fprintf(stderr, "could not prepare output or kernel\n");
        image_destroy(in);
        image_destroy(out);
        kernel_destroy(kernel);
        return 1;
    }
    rc = conv2d_dispatch(in, out, kernel, &params);
    if (rc == 0 && image_save_pgm(out, opt->out_path) != 0) {
        fprintf(stderr, "could not write %s\n", opt->out_path);
        rc = -1;
    }
    image_destroy(in);
    image_destroy(out);
    kernel_destroy(kernel);
    return rc == 0 ? 0 : 1;
}

static int cmd_conv3d(const Options *opt) {
    ConvParams params;
    Volume *in;
    Volume *out;
    Kernel *kernel;
    int rc;
    if (params_from_options(opt, &params) != 0 || opt->in_path == NULL || opt->out_path == NULL ||
        opt->size < 1) {
        fprintf(stderr, "conv3d needs --mode, --in, --out, and --size\n");
        return 1;
    }
    in = volume_load(opt->in_path);
    if (in == NULL) {
        fprintf(stderr, "could not read %s\n", opt->in_path);
        return 1;
    }
    out = volume_create(in->width, in->height, in->depth);
    kernel = kernel_create(3, opt->size);
    if (out == NULL || kernel == NULL || kernel_fill_box(kernel) != 0) {
        fprintf(stderr, "could not prepare output or kernel\n");
        volume_destroy(in);
        volume_destroy(out);
        kernel_destroy(kernel);
        return 1;
    }
    rc = conv3d_dispatch(in, out, kernel, &params);
    if (rc == 0 && volume_save(out, opt->out_path) != 0) {
        fprintf(stderr, "could not write %s\n", opt->out_path);
        rc = -1;
    }
    volume_destroy(in);
    volume_destroy(out);
    kernel_destroy(kernel);
    return rc == 0 ? 0 : 1;
}

static int cmd_median2d(const Options *opt) {
    ConvParams params;
    Image *in;
    Image *out;
    if (params_from_options(opt, &params) != 0 || opt->in_path == NULL || opt->out_path == NULL ||
        opt->window < 1) {
        fprintf(stderr, "median2d needs --mode, --in, --out, and --window\n");
        return 1;
    }
    if (params.mode != CONV_SEQ && params.mode != CONV_TILES) {
        fprintf(stderr, "median2d mode must be seq or tiles\n");
        return 1;
    }
    in = image_load_pgm(opt->in_path);
    out = in != NULL ? image_create(in->width, in->height) : NULL;
    if (in == NULL || out == NULL) {
        fprintf(stderr, "could not read %s\n", opt->in_path);
        image_destroy(in);
        image_destroy(out);
        return 1;
    }
    convoscale_status_reset();
    if (params.mode == CONV_SEQ) {
        median2d_seq(in, out, opt->window);
    } else {
        median2d_tiles(in, out, opt->window, params.threads, params.tile);
    }
    if (convoscale_status() == 0 && image_save_pgm(out, opt->out_path) != 0) {
        convoscale_fail("could not write output");
    }
    image_destroy(in);
    image_destroy(out);
    return convoscale_status() == 0 ? 0 : 1;
}

static int cmd_filter(const char *name, const Options *opt) {
    ConvParams params;
    Image *in2 = NULL;
    Image *out2 = NULL;
    Volume *in3 = NULL;
    Volume *out3 = NULL;
    int is_3d = 0;
    int rc;

    if (name == NULL || params_from_options(opt, &params) != 0) {
        return 1;
    }
    is_3d = strcmp(name, "gaussian3d") == 0 || strcmp(name, "mean3d") == 0 ||
            strcmp(name, "sobel3d") == 0;
    if (opt->in_path == NULL || opt->out_path == NULL) {
        fprintf(stderr, "filter needs --in and --out\n");
        return 1;
    }
    if (is_3d) {
        in3 = volume_load(opt->in_path);
        out3 = in3 != NULL ? volume_create(in3->width, in3->height, in3->depth) : NULL;
        if (in3 == NULL || out3 == NULL) {
            fprintf(stderr, "could not read %s\n", opt->in_path);
            volume_destroy(in3);
            volume_destroy(out3);
            return 1;
        }
    } else {
        in2 = image_load_pgm(opt->in_path);
        out2 = in2 != NULL ? image_create(in2->width, in2->height) : NULL;
        if (in2 == NULL || out2 == NULL) {
            fprintf(stderr, "could not read %s\n", opt->in_path);
            image_destroy(in2);
            image_destroy(out2);
            return 1;
        }
    }

    if (strcmp(name, "gaussian2d") == 0) {
        rc = filter_gaussian2d(in2, out2, opt->size, opt->sigma, &params);
    } else if (strcmp(name, "box2d") == 0) {
        rc = filter_box2d(in2, out2, opt->size, &params);
    } else if (strcmp(name, "sharpen2d") == 0) {
        rc = filter_sharpen2d(in2, out2, &params);
    } else if (strcmp(name, "emboss2d") == 0) {
        rc = filter_emboss2d(in2, out2, &params);
    } else if (strcmp(name, "sobel2d") == 0) {
        rc = filter_sobel2d(in2, out2, &params);
    } else if (strcmp(name, "prewitt2d") == 0) {
        rc = filter_prewitt2d(in2, out2, &params);
    } else if (strcmp(name, "laplacian2d") == 0) {
        rc = filter_laplacian2d(in2, out2, &params);
    } else if (strcmp(name, "gaussian3d") == 0) {
        rc = filter_gaussian3d(in3, out3, opt->size, opt->sigma, &params);
    } else if (strcmp(name, "mean3d") == 0) {
        rc = filter_mean3d(in3, out3, opt->size, &params);
    } else if (strcmp(name, "sobel3d") == 0) {
        rc = filter_sobel3d(in3, out3, &params);
    } else {
        fprintf(stderr, "unknown filter %s\n", name);
        rc = -1;
    }

    if (rc == 0) {
        if (is_3d) {
            rc = volume_save(out3, opt->out_path);
        } else {
            rc = image_save_pgm(out2, opt->out_path);
        }
        if (rc != 0) {
            fprintf(stderr, "could not write %s\n", opt->out_path);
        }
    }
    image_destroy(in2);
    image_destroy(out2);
    volume_destroy(in3);
    volume_destroy(out3);
    return rc == 0 ? 0 : 1;
}

static int cmd_cmp(const Options *opt) {
    Image *a;
    Image *b;
    int same;
    if (opt->path_a == NULL || opt->path_b == NULL) {
        fprintf(stderr, "cmp needs --a and --b\n");
        return 1;
    }
    a = image_load_pgm(opt->path_a);
    b = image_load_pgm(opt->path_b);
    if (a == NULL || b == NULL) {
        fprintf(stderr, "could not read both images\n");
        image_destroy(a);
        image_destroy(b);
        return 1;
    }
    same = image_equal(a, b);
    printf("%s\n", same ? "MATCH" : "MISMATCH");
    image_destroy(a);
    image_destroy(b);
    return same ? 0 : 1;
}

static int cmd_cmp3d(const Options *opt) {
    Volume *a;
    Volume *b;
    int same;
    if (opt->path_a == NULL || opt->path_b == NULL) {
        fprintf(stderr, "cmp3d needs --a and --b\n");
        return 1;
    }
    a = volume_load(opt->path_a);
    b = volume_load(opt->path_b);
    if (a == NULL || b == NULL) {
        fprintf(stderr, "could not read both volumes\n");
        volume_destroy(a);
        volume_destroy(b);
        return 1;
    }
    same = volume_equal(a, b);
    printf("%s\n", same ? "MATCH" : "MISMATCH");
    volume_destroy(a);
    volume_destroy(b);
    return same ? 0 : 1;
}

static int cmd_check(const Options *opt) {
    Image *got;
    Image *ref;
    size_t mismatches = 0;
    int rc;
    if (opt->path_a == NULL || opt->path_b == NULL) {
        fprintf(stderr, "check needs --a GOT and --b REF\n");
        return 1;
    }
    got = image_load_pgm(opt->path_a);
    ref = image_load_pgm(opt->path_b);
    if (got == NULL || ref == NULL) {
        fprintf(stderr, "could not read both images\n");
        image_destroy(got);
        image_destroy(ref);
        return 1;
    }
    rc = check_image(got, ref, &mismatches);
    image_destroy(got);
    image_destroy(ref);
    return rc == 0 ? 0 : 1;
}

static int cmd_benchgen(const Options *opt) {
    int rc;
    if (opt->out_path == NULL || opt->width < 1 || opt->height < 1) {
        fprintf(stderr, "benchgen needs --width, --height, and --out\n");
        return 1;
    }
    if (opt->depth > 0) {
        Volume *vol = volume_create(opt->width, opt->height, opt->depth);
        if (vol == NULL) {
            fprintf(stderr, "could not allocate volume\n");
            return 1;
        }
        rc = generate_benchmark_volume(vol);
        if (rc == 0) {
            rc = volume_save(vol, opt->out_path);
        }
        volume_destroy(vol);
        return rc == 0 ? 0 : 1;
    }
    {
        Image *img = image_create(opt->width, opt->height);
        if (img == NULL) {
            fprintf(stderr, "could not allocate image\n");
            return 1;
        }
        rc = generate_benchmark_image(img);
        if (rc == 0) {
            rc = image_save_pgm(img, opt->out_path);
        }
        image_destroy(img);
        return rc == 0 ? 0 : 1;
    }
}

static int cmd_batch(const Options *opt) {
    int threads = opt->threads > 0 ? opt->threads : omp_get_max_threads();
    double sequential_seconds = 0.0;
    double parallel_seconds = 0.0;
    int rc;
    if (opt->directory == NULL) {
        fprintf(stderr, "batch needs --dir\n");
        return 1;
    }
    if (threads < 1) {
        threads = 1;
    }
    rc = batch_run(opt->directory, threads, &sequential_seconds, &parallel_seconds);
    if (rc == 0) {
        printf("sequential %.6f\nparallel %.6f\n", sequential_seconds, parallel_seconds);
    }
    return rc == 0 ? 0 : 1;
}

int main(int argc, char **argv) {
    Options opt;
    const char *command;
    options_init(&opt);
    if (argc < 2 || strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "help") == 0) {
        usage();
        return argc < 2 ? 1 : 0;
    }
    command = argv[1];
    if (strcmp(command, "info") == 0) {
        return cmd_info();
    }
    if (strcmp(command, "self-test") == 0) {
        return cmd_self_test();
    }
    if (strcmp(command, "filter") == 0) {
        if (argc < 3 || parse_options(argc, argv, 3, &opt) != 0) {
            usage();
            return 1;
        }
        return cmd_filter(argv[2], &opt);
    }
    if (parse_options(argc, argv, 2, &opt) != 0) {
        return 1;
    }
    if (strcmp(command, "generate2d") == 0) {
        return cmd_generate2d(&opt);
    }
    if (strcmp(command, "generate3d") == 0) {
        return cmd_generate3d(&opt);
    }
    if (strcmp(command, "conv2d") == 0) {
        return cmd_conv2d(&opt);
    }
    if (strcmp(command, "conv3d") == 0) {
        return cmd_conv3d(&opt);
    }
    if (strcmp(command, "median2d") == 0) {
        return cmd_median2d(&opt);
    }
    if (strcmp(command, "cmp") == 0) {
        return cmd_cmp(&opt);
    }
    if (strcmp(command, "cmp3d") == 0) {
        return cmd_cmp3d(&opt);
    }
    if (strcmp(command, "check") == 0) {
        return cmd_check(&opt);
    }
    if (strcmp(command, "benchgen") == 0) {
        return cmd_benchgen(&opt);
    }
    if (strcmp(command, "batch") == 0) {
        return cmd_batch(&opt);
    }
    fprintf(stderr, "unknown command %s\n", command);
    usage();
    return 1;
}
