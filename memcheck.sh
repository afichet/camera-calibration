valgrind ./build/bin/extract-patches data/measurements/Colors_0000000320_dem.tiff data/measurements/boxes.csv output/patches.csv
valgrind ./build/bin/gen-ref-colorchart data/D65.csv data/XYZ.csv output/reference.csv
valgrind ./build/bin/gen-colorchart-image output/reference.csv output/reference.exr
valgrind ./build/bin/extract-matrix output/reference.csv output/patches.csv output/matrix.csv
valgrind ./build/bin/correct-patches output/patches.csv output/matrix.csv output/corrected_patches.csv
valgrind ./build/bin/correct-image data/measurements/Colors_0000000320_dem.tiff output/matrix.csv output/corrected_image.tiff
