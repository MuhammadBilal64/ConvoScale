# ConvoScale

Large-scale 2D and 3D image convolution with OpenMP. Sequential, row-wise, column-wise, and tile-based versions share one API. Task ownership is in `task_division.md`.

## Build

GCC with OpenMP is required. On Windows, use the lab machine or WSL for timed runs. Building here with MSYS2 GCC is enough to compile.

```bash
make
make test
```

The binary is `build/convoscale` (`build/convoscale.exe` on Windows). If `make` is not on PATH in PowerShell, run these commands in WSL, or install Make in MSYS2 with `pacman -S make`.

```bash
./build/convoscale info
./build/convoscale self-test
```

## Rules the code already follows

- Row-major `uint8` pixels. Accumulate in `float`, then store with `conv_to_u8` (nearest, clamped to 0–255).
- `image_pixel` and `volume_pixel` return 0 outside the data. Output size matches the input.
- A 2D kernel has odd `size` from 1 to 31. Weights are row-major, length `size * size`. A 3D kernel has length `size * size * size`.
- Volume index is `x + width * (y + height * z)`.
- `conv2d` / `conv3d` pass a uniform box kernel. Named filters live in `src/filters_*.c`.
- Sobel and Prewitt are two convolutions combined per pixel. Median does not use `Kernel`.
- Speedup CSVs use the columns in `task_division.md`. `scripts/plot_results.py` reads them.

## Commands

```bash
./build/convoscale generate2d --width 512 --height 512 --out data/test.pgm
./build/convoscale generate3d --width 64 --height 64 --depth 64 --out data/test.cvol
./build/convoscale conv2d --mode seq --in data/test.pgm --out data/out.pgm --size 3
./build/convoscale conv2d --mode rows --in data/test.pgm --out data/out.pgm --size 3 --threads 4 --schedule dynamic --chunk 16
./build/convoscale filter gaussian2d --mode seq --in data/test.pgm --out data/out.pgm --size 5 --sigma 1.2
./build/convoscale filter sobel2d --mode cols --in data/test.pgm --out data/out.pgm --threads 4
./build/convoscale median2d --mode tiles --in data/test.pgm --out data/out.pgm --window 3 --tile 32
./build/convoscale cmp --a data/out.pgm --b data/ref.pgm
./build/convoscale --help
```

`generate2d` and `generate3d` write a deterministic test pattern so parallel code can be tried before the benchmark generator exists. `benchgen` calls Ashhad's generator. `check` calls his checker. `cmp` is a byte compare that works now.

Modes are `seq`, `rows`, `cols`, and `tiles`. Defaults are the core count, `static`, chunk `0`, tile `32`, and sigma `1`.

Volume files start with `CVOL`, then `width height depth`, then raw `uint8` bytes.

## Where to write

Replace the stub body and keep the signature. A stub prints `not implemented` and the command exits with status 1.

| File | Owner | Fills in |
|---|---|---|
| `src/conv2d_seq.c`, `src/conv3d_seq.c`, `src/kernel.c` | Ashhad | Naive convolution and Gaussian weights |
| `src/filters_smooth.c`, `src/generate.c`, `src/check.c` | Ashhad | Smoothing filters, benchmark data, checker |
| `src/conv2d_rows.c`, `src/conv3d_slices.c` | Saad | Row-wise and slice-wise OpenMP |
| `src/filters_sharpen.c`, `src/batch.c` | Saad | Sharpen, emboss, DIV2K batch |
| `src/conv2d_cols.c`, `src/conv3d_cols.c`, `src/filters_edge.c` | Bilal | Column-wise OpenMP and edge filters |
| `src/conv2d_tiles.c`, `src/conv3d_tiles.c`, `src/median.c` | Shaheer | Tiles, cache blocking, median |

`include/`, `src/main.c`, `src/image.c`, `src/dispatch.c`, `src/kernel_util.c`, and `scripts/plot_results.py` are the shared skeleton. `kernel_fill_box` and `kernel_fill_mean` are already implemented. `kernel_fill_gaussian` is not.

Call `conv2d_dispatch` or `conv3d_dispatch` from a filter so the `--mode` flag selects sequential, rows, columns, or tiles. `threads` is at least 1 when it comes from dispatch. `chunk` 0 means the OpenMP default.

## Plots

```bash
python scripts/plot_results.py results/rows.csv --sequential results/sequential.csv --x threads --out results/plots
```

`--x` is `threads`, `width`, `kernel`, or `tile`. The script averages repeats, then writes time, speedup, and efficiency PNGs. Speedup uses the sequential row with the same dimension, filter, size, and kernel.
