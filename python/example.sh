#!/bin/sh

mkdir output

./extract_patches.py \
    --image ../measurements/Colors_0000000320_dem.tiff \
    --areas ../measurements/boxes.csv \
    --output output/patches.csv

./extract_matrix.py \
    --reference data/macbeth_patches.csv \
    --illuminant data/D65.csv \
    --cmf data/XYZ.csv \
    --camera output/patches.csv \
    --output output/d65_xyz_matrix.csv

cd src
g++ main.cpp -o ../correct -O2 -fopenmp -lpthread

cd ..
./correct \
    measurements/Colors_0000000320_dem.tiff \
    output/d65_xyz_matrix.csv \
    output/corrected.tiff
