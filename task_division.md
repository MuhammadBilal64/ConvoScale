# Task Division (final)

**Project:** Large-Scale 2D and 3D Image Convolution Using OpenMP  
**Course:** Parallel and Distributed Computing  
**Repo:** ConvoScale  
**Status:** Final. Same four owners. Scope below is the required work.

| Member | Roll no. | Owns |
|---|---|---|
| Syed Ashhad Hassan | 23K-0584 | Sequential baseline, generator, smoothing filters, correctness |
| Saad Shakeel | 23K-0614 | Row-wise OpenMP, scheduling, sharpening filters, batch mode |
| Muhammad Bilal | 23K-0026 | Repository setup, column-wise OpenMP, edge filters, comparison graphs |
| Shaheer Khan | 23K-0793 | Tile-wise OpenMP, cache blocking, median filter |

Each person implements their strategy, times it, and writes that report section, then walks the other three through it. The viva still requires every member to explain the whole project.

## Locked decisions

1. GCC with `-fopenmp`. All timed CSVs come from the same machine (lab Linux or WSL).
2. Row-major `uint8` images and volumes. A 20,000×20,000 image is about 400 MB. Accumulate in `float`, round to nearest `uint8`, and clamp to 0–255. Correctness is an exact byte match.
3. Zero-padding. Output size equals input size in every mode.
4. Timed linear filters use naive convolution. A 31×31 kernel is 961 multiplies per pixel. Separable Gaussian or box blur stays out of the measured runs.
5. Median is its own function. On large images the windows are 3, 5, 7, and 9. A 31×31 median is out of the timing matrix.
6. One sequential CSV is the speedup denominator. Ashhad publishes `results/sequential.csv`. Sequential time does not depend on threads, schedule, or tile, so each (filter, size, kernel) is timed once.
7. Full byte-for-byte checks use a 512² image and a 64³ volume. Larger outputs use a checksum and a sample of pixels.
8. The schedule study is 2D row-wise only, on the thread-scaling image. 3D slices, columns, and tiles use `static`.
9. Batch required run: the DIV2K training set (800 images), or the most the machine can hold, with a floor of 100. The report states the exact count. These times stay out of the row-vs-column-vs-tile graphs.
10. SIMD is optional. Shaheer’s required work ends at scalar tiles, the tile-size sweep, median, and the conclusions. SIMD is reported only if the required CSVs are already in.

Speedup = sequential time / parallel time. Efficiency = speedup / thread count. Five repeats, then the average. Threads: 1, 2, 4, 8, and the core count. Time with `omp_get_wtime`.

```c
typedef enum { SCHED_STATIC, SCHED_DYNAMIC, SCHED_GUIDED } Schedule;

void conv2d_seq(const Image *in, Image *out, const Kernel *k);
void conv2d_rows(const Image *in, Image *out, const Kernel *k,
                 int threads, Schedule sched, int chunk);
void conv2d_cols(const Image *in, Image *out, const Kernel *k, int threads);
void conv2d_tiles(const Image *in, Image *out, const Kernel *k,
                  int threads, int tile);

void conv3d_seq(const Volume *in, Volume *out, const Kernel *k);
void conv3d_slices(const Volume *in, Volume *out, const Kernel *k, int threads);
void conv3d_cols(const Volume *in, Volume *out, const Kernel *k, int threads);
void conv3d_tiles(const Volume *in, Volume *out, const Kernel *k,
                  int threads, int tile);

void median2d_seq(const Image *in, Image *out, int window);
void median2d_tiles(const Image *in, Image *out, int window,
                    int threads, int tile);
```

`chunk` 0 means the OpenMP default. CSV columns:

```text
dim,mode,filter,width,height,depth,kernel,threads,schedule,chunk,tile,seconds,repeat
```

`depth` is 1 for 2D. Unused fields are `na`.

## Workload

Filter counts differ. The required effort is the same kind of package for each person: one strategy, the filters that belong to it, one extra system piece, one report section.

| | Ashhad | Saad | Bilal | Shaheer |
|---|---|---|---|---|
| Strategy | Sequential reference | Rows / slices | Columns | Tiles |
| Filters | Gaussian, box, 3D Gaussian, 3D mean | Sharpen, emboss | Sobel, Prewitt, Laplacian, 3D Sobel | Median |
| Extra piece | Generator and checker | Schedules and batch | Repo setup and comparison graphs | Cache blocking and tile-size sweep |
| Report | Design and correctness | Scheduling and batch | Memory and comparison | Tiling and conclusions |
| Optional | 30,000² and a volume above 512³, if memory allows | — | — | SIMD on one linear filter |

## Files

| Path | Owner |
|---|---|
| `Makefile`, `.gitignore`, `README.md`, `include/`, `src/main.c`, `src/image.c` | Bilal |
| `src/dispatch.c`, `src/kernel_util.c`, `scripts/plot_results.py` | Bilal |
| `src/conv2d_cols.c`, `src/conv3d_cols.c`, `src/filters_edge.c` | Bilal |
| `src/conv2d_seq.c`, `src/conv3d_seq.c`, `src/kernel.c`, `src/generate.c` | Ashhad |
| `src/filters_smooth.c`, `src/check.c` | Ashhad |
| `src/conv2d_rows.c`, `src/conv3d_slices.c`, `src/filters_sharpen.c`, `src/batch.c` | Saad |
| `src/conv2d_tiles.c`, `src/conv3d_tiles.c`, `src/median.c` | Shaheer |
| `results/sequential.csv` | Ashhad |
| `results/rows.csv`, `results/schedules.csv`, `results/batch.csv` | Saad |
| `results/cols.csv` | Bilal |
| `results/tiles.csv` | Shaheer |

The skeleton compiles before the algorithms exist. `kernel_fill_box` and `kernel_fill_mean` are in `src/kernel_util.c`. `src/kernel.c` is Gaussian only.

## Syed Ashhad Hassan (23K-0584)

Start as soon as the headers and `make` exist. The other three check their code against this.

- Generic sequential 2D and 3D convolution. Linear filters only swap the kernel.
- Kernel builder: dimension, odd size, sigma, box and mean weights. One builder for every size.
- Filters: 2D Gaussian, box blur, 3D Gaussian, 3D mean. A before/after PGM pair for each.
- Generator, deterministic: 5,000, 10,000, and 20,000 square images. Volumes at 128³, 256³, and 512³. 30,000² and a volume above 512³ only when the timing machine has memory. 128³ and 256³ exist so the 3D plots have more than one point. 512³ is the required volume.
- Checker: exact `uint8` match on the small sizes, checksum plus sample on large sizes. The others run it on their own code first. Ashhad records pass or fail for the report.
- `results/sequential.csv` for every (filter, size, kernel) in the experiment table. Freeze it once published.
- Report: introduction, objectives, dataset, sequential design, correctness.

**Done when** every linear filter runs through `conv2d_seq` / `conv3d_seq` by changing the kernel, the generator writes the required sizes, and `results/sequential.csv` plus a pass/fail note for each parallel mode are in the repo.

## Saad Shakeel (23K-0614)

- `conv2d_rows`: `#pragma omp parallel for` over rows, with `schedule(static)`, `schedule(dynamic)`, and `schedule(guided)`.
- Chunk study, 2D only, on the thread-scaling image: dynamic with chunk 1 and chunk 16. Static and guided use the default chunk.
- `conv3d_slices`: parallel for over slices, `static` only.
- Thread counts 1, 2, 4, 8, and the core count.
- Sharpen and emboss, as kernels on Ashhad’s sequential function and on the row-wise function. A before/after pair for each.
- Batch: a directory reader, sequential loop versus one image per thread. Required run is DIV2K training (800), or the largest subset that fits, at least 100. Record the count.
- CSVs: `results/rows.csv`, `results/schedules.csv`, `results/batch.csv`.
- Report: why rows are the direct split, what the schedules changed, load balance, where extra threads stop helping, and the batch result.
- Viva command: one binary, one image, a thread count the demonstrator can change.

**Done when** row-wise and slice-wise match sequential on a small input for sharpen and emboss, the schedule table exists at more than one thread count, and batch reports both times.

## Muhammad Bilal (23K-0026)

Repository first, so the others compile against one API.

- Directories `include/`, `src/`, `scripts/`, `data/`, `results/`.
- `Makefile`: `gcc -std=c11 -O2 -Wall -Wextra -fopenmp`, `clean`, executable name `convoscale`.
- `.gitignore` for build outputs, generated images, and CSV results.
- README with build steps, run steps, and the locked decisions above.
- `Image`, `Volume`, `Kernel`, allocate, free, row-major index, zero-padding read, PGM read/write.
- `src/main.c` selects `seq`, `rows`, `cols`, or `tiles`, and passes threads, schedule, chunk, and tile size.
- Stub sources for Ashhad, Saad, and Shaheer.
- `scripts/plot_results.py` reads the CSV columns and draws time, speedup, and efficiency.

Then the column-wise work:

- `conv2d_cols`: each thread takes a group of columns.
- `conv3d_cols`: parallelize along the width.
- Measure the gap against Saad’s row-wise run on the same image, kernel, filter, and thread count.
- Report cache misses from the stride, and false sharing when adjacent output columns share a cache line. Document any padding or private region you add.
- Filters: Sobel, Prewitt, Laplacian, 3D Sobel. A before/after pair for each.
- `results/cols.csv` from the experiment table.
- Head-to-head graphs for 2D and 3D: row vs column vs tile, time, speedup, and efficiency, against threads, data size, and kernel size. Inputs are the four CSVs above. Use Shaheer’s chosen tile size.
- Report: memory layout, column-wise results, bandwidth, and why speedup flattens.

**Done when** `make` builds with OpenMP, column-wise matches sequential on a small image for every edge filter, and the comparison graphs include all three splits in 2D and 3D.

## Shaheer Khan (23K-0793)

- `conv2d_tiles` with `collapse(2)`, and `conv3d_tiles`.
- Tile edges 16, 32, 64, 128, and 256. Cache blocking is this sweep: the tile has to keep the neighborhood in cache. Send the best edge to Bilal for the comparison graphs.
- `median2d_seq` and `median2d_tiles`. Windows 3, 5, 7, and 9 on large images. A before/after pair on a noisy image.
- `results/tiles.csv`, including the tile-size graph data.
- Report: tiling, tile-size results, cache blocking, and how median scales compared with Gaussian. Conclusions and limitations come last, after the other three report sections, and use the same numbers as the graphs.
- Optional, after `results/tiles.csv` is in the comparison set: SIMD the inner loop of box blur or Gaussian with `omp simd` or GCC vectors. The main table stays on the scalar tile code.

**Done when** tiled 2D and 3D match sequential on a small input, the tile-size graph exists, the chosen tile size is the one in Bilal’s comparison, and tiled median matches `median2d_seq`.

## Experiment table

Five repeats. Ashhad’s sequential time is the denominator. The filters in the timing matrix are Gaussian, Sobel, and median, plus sharpen where a run needs one filter from each family.

| Experiment | Fixed | Varies | Owner |
|---|---|---|---|
| Correctness | 512² and 64³ | All filters, all four modes | Each owner, then Ashhad |
| Thread scaling | 10,000² and 512³. 2D kernel 15, 3D kernel 7. Median window 5 | Threads 1, 2, 4, 8, max. Filters: Gaussian, Sobel, median | Strategy owner |
| Schedules | Row-wise, 10,000², kernel 15, Gaussian | `static`, `dynamic` chunk 1, `dynamic` chunk 16, `guided` | Saad |
| Data size | Core count, kernel 15, median window 5 | 5,000, 10,000, 20,000. One filter per family | Each strategy owner runs their mode. Bilal plots them |
| Kernel size | 10,000², core count. 3D: 256³ | Linear: 3, 7, 15, 31. 3D linear: 3, 7, 11. Median: 3, 5, 7, 9 | Each strategy owner |
| Tile size | 10,000², core count, Gaussian, kernel 15 | 16, 32, 64, 128, 256 | Shaheer |
| Batch | DIV2K directory, floor 100 images | Sequential loop vs one image per thread | Saad |
| Optional | Same filter and kernel as thread scaling | 30,000² or a volume above 512³ | Ashhad generates it. Strategy owners run it only if it fits |

A 20,000 image with a 31×31 kernel is a multi-minute single-core run. Script the table and run it overnight. Extra combinations stay out.

## Order

1. Bilal: Makefile, headers, stubs, README. The others can compile in the first work session.
2. Ashhad: sequential convolution, kernel builder, Gaussian, generator. A 512² image can be filtered and saved.
3. Saad, Bilal, and Shaheer: their splits. A small-image match against sequential comes before any 10,000² timing.
4. Ashhad publishes `results/sequential.csv`.
5. Saad, Bilal, and Shaheer publish their CSVs. Bilal draws the comparison graphs.
6. Report sections. Shaheer writes the conclusions last. Ashhad reads the full report once so the padding rule, the speedup definition, and the numbers match.
7. Saad’s viva command. Every member runs it once.

## Report sections

| Section | Author |
|---|---|
| Introduction, objectives, dataset, sequential design | Ashhad |
| Correctness, with before/after images from each owner | Ashhad |
| Row-wise, scheduling, load balance, batch | Saad |
| Column-wise, cache, bandwidth, row vs column vs tile | Bilal |
| Tiles, cache blocking, median, conclusions | Shaheer |
| SIMD, only if the optional run exists | Shaheer |

## Viva

Each person can do all of the following on the whole project:

- Build it and run sequential, row, column, and tile modes at two thread counts.
- Compute speedup and efficiency from the numbers on the slide.
- Explain the column-wise stride cost, and why a mid-size tile can beat a whole-row split.
- State the padding rule, the pixel type, and why median is a separate function.
