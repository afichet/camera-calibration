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
