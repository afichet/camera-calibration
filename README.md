# Compilation

You need f2c (`libf2c2-dev` on Ubuntu 20.04LTS), BLAS / LAPACK. See
http://users.ics.forth.gr/~lourakis/levmar/

Then,

```bash
mkdir build
cmake ..
make
```

# Pipeline example

You still need the python utility `extract_patches.py` in `python` for
extracting measured Macbeth patches from TIFF image.

```bash
./python/extract_patches.py \
    --image data/measurements/Colors_0000000320_dem.tiff \
    --areas data/measurements/boxes.csv \
    --output data/measurements/patches.csv
```

Then, you need to generate a reference set of Macbeth patches given an
illuminant and color matching functions:

```bash
mkdir output

./build/bin/gen-ref-colorchart \
    data/D65.csv \
    data/XYZ.csv \
    output/reference.csv
```

Now, let's see how this reference colour chart looks like:

```bash
./build/bin/gen-colorchart-image \
    output/reference.csv \
    output/reference.png
```

And how the averaged measured one looks like:

```bash
./build/bin/gen-colorchart-image \
    patches.csv \
    output/orig_measured.png
```

Now, we want to find the matrix which transform the measured color to
the reference colors:

```bash
./build/bin/extract-matrix \
    output/reference.csv \
    patches.csv \
    output/matrix.csv
```

Finally, we correct the measured patches:

```bash
./build/bin/correct-patches \
    patches.csv \
    output/matrix.csv \
    output/corrected_patches.csv
```

And, create an image of the corrected patches:

```bash
./build/bin/gen-colorchart-image \
    output/corrected_patches.csv \
    output/corrected_measured.png
```
