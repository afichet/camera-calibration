Building the toolkit
====================

First, checkout the repository:

```bash
git clone https://github.com/afichet/camera-calibration.git
cd camera-calibration
git submodule init
git submodule update
```

## Dependencies

On all platforms, you need CMake, a C and a C++ compiler.

### Windows and macOS

We compile levmar without BLAS / LAPACK to have simple compilation
pipeline on those platforms. You can re-enable it in the
`external/levmar-2.6/CMakeLists.txt` for better performances.

### Linux

You need f2c (`libf2c2-dev` on Ubuntu 20.04LTS), BLAS / LAPACK. See
http://users.ics.forth.gr/~lourakis/levmar/

## Compilation

```bash
mkdir build
cd build
cmake ..
make
```


Images
======

We currently support TIFF and EXR images for reading and
writting. TIFF are supported only if `libtiff` is available on the
system.


Applications
============

## Colourotron

`Colourotron` is a GUI application to fit a correction matrix from a shot of a 
Macbeth colourchecker. The source code is in the `app/gui` folder.

## Extract patches

`extract-patches` is used for creating a CSV file from an image and a
CSV containing the areas to average. The CSV file contains the top
left pixel and bottom right pixel for the box to extract.

```csv
<top left x>, <top left y>, <bottom left x>, <bottom right y>
...
```

You can control which area are averaged using the `overlay-areas`
utility.

## Overlay areas

`overlay-areas` is used to display an overlay of the area that are
going to be averaged by `extract-patches` utility.

## Generate reference colorchart

`gen-ref-colorchart` is used to create a CSV file containing the XYZ
values of each color patch from reference measured data (available
here: https://www.babelcolor.com/colorchecker-2.htm) given an
illuminant and color matching functions (CMFs).

## Generate colorchart image

`gen-colorchart-image` creates a PNG preview of a given CSV file
containing color values of each patch of a Macbeth colorchecker.

## Fitting

`extract-matrix` fits a transformation matrix from a measured set of
colors to a reference set of colors.

## Correct patches

`correct-patches` corrects color values of each patches contained in a
CSV file using the provided transformation matrix.

## Correct image

`correct-image` corrects color values of each pixel from a TIFF or EXR
file using the provided transformation matrix.

## Conversion to DNG

`raw-to-dng` handles the propriatery RAW format this application targets and
outputs a standard DNG file. It requires a calibration matrix that can be
extracted using `Colourotron` fitting feature.


Pipeline example
================

We provide a pipeline example in `run.sh` script.

## GUI

### Colour calibration

This is the recommended solution.

Open your image with `Colourotron`. 
1- Move the Macbeth outline to match the captured Macbeth.
2- Click on `Fit...`. 
3- Click on `Apply`.
4- Save the matrix with `File -> Save correction matrix...`

This matrix can then be used with the command line tools.

### Demosaicing

To perform colour correction, you either can convert the propriatery RAW to DNG
or first debayer the image and apply the correction matrix. To debayer the 
image, use `derawzinator`:

```bash
./build/bin/derawzinator \
    raw_image_INFO.txt \
    raw_image.exr
```

### Colour correction

We can correct any given RGB image using the matrix:

```bash
./build/bin/correct-image \
    data/measurements/Colors_0000000320_dem.tiff \
    output/matrix.csv \
    output/corrected_image.tiff
```

You can also convert the propriatery RAW format to DNG:

```bash
./build/bin/raw-to-dng \
    raw_image_INFO.txt \
    output/matrix.csv \
    output/output.dng
```

## Command line

First, you need to extract from a TIFF file the values of the Macbeth
color checker. You need as well a CSV file (comma separated)
describing each area to average.

```bash
mkdir output

./build/bin/extract-patches
    data/measurements/Colors_0000000320_dem.tiff \
    data/measurements/boxes.csv \
    output/patches.csv
```

Then, you need to generate a reference set of Macbeth patches given an
illuminant and color matching functions:

```bash
./build/bin/gen-ref-colorchart \
    data/D65.csv \
    data/XYZ.csv \
    output/reference.csv
```

Now, let's see how this reference colour chart looks like:

```bash
./build/bin/gen-colorchart-image \
    output/reference.csv \
    output/reference.exr
```

And how the averaged measured one looks like:

```bash
./build/bin/gen-colorchart-image \
    output/patches.csv \
    output/orig_measured.exr \
	true
```

Now, we want to find the matrix which transform the measured color to
the reference colors:

```bash
./build/bin/extract-matrix \
    output/reference.csv \
    output/patches.csv \
    output/matrix.csv
```

Finally, we correct the measured patches:

```bash
./build/bin/correct-patches \
    output/patches.csv \
    output/matrix.csv \
    output/corrected_patches.csv
```

And, create an image of the corrected patches:

```bash
./build/bin/gen-colorchart-image \
    output/corrected_patches.csv \
    output/corrected_measured.exr
```

We also can correct any given image using the matrix, here the input
image:

```bash
./build/bin/correct-image \
    data/measurements/Colors_0000000320_dem.tiff \
    output/matrix.csv \
    output/corrected_image.tiff
```

You can also convert the propriatery RAW format to DNG:

```bash
./build/bin/raw-to-dng \
    raw_image_INFO.txt \
    output/matrix.csv \
    output/output.dng
```