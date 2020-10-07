# Compilation

You need f2c (`libf2c2-dev` on Ubuntu 20.04LTS), BLAS / LAPACK. See http://users.ics.forth.gr/~lourakis/levmar/

Then,

```bash
mkdir build
cmake ..
make
```

# Pipeline example

You still need the python utility `extract_patches.py` in `python` for extracting measured Macbeth patches from TIFF image.

```bash
./python/extract_patches.py \
    --image measurements/Colors_0000000320_dem.tiff \
    --areas measurements/boxes.csv \
    --output output/patches.csv
```

Then, you need to generate a reference set of Macbeth patches given an illuminant and color matching functions:

```bash
./build/bin/gen-ref-colorchart \
    data/D65.csv \
    data/XYZ.csv \
    output/reference.csv
```

```bash

./build/bin/gen-colorchart-image \
    output/reference.csv \
    output/reference.png

./build/bin/gen-colorchart-image \
    patches.csv \
    output/orig_measured.png

./build/bin/extract-matrix \
    output/reference.csv \
    patches.csv \
    output/matrix.csv

./build/bin/correct-patches \
    patches.csv \
    output/matrix.csv \
    output/corrected_patches.csv

./build/bin/gen-colorchart-image \
    output/corrected_patches.csv \
    output/corrected_measured.png
```
