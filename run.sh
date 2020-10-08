#!/bin/sh

image_measurement=data/measurements/Colors_0000000320_dem.tiff
bounds_measurement=data/measurements/boxes.csv
ref_illu=data/D65.csv
ref_cmf=data/XYZ.csv

bin_dir=./build/bin/

mkdir output


###############################################################################
# Measurement extraction
###############################################################################

# Extract patches from the measured image
${bin_dir}extract-patches \
	${image_measurement} \
	${bounds_measurement} \
	output/patches.csv

# Show areas averaged for each patch
${bin_dir}overlay-areas \
	${image_measurement} \
	${bounds_measurement} \
	output/orig_overlays.tiff

# Create a preview of the measured patches
${bin_dir}gen-colorchart-image \
    output/patches.csv \
    output/orig_measured.png \
	true


###############################################################################
# Reference generation
###############################################################################

# Generate reference patches for given illuminant & CMFs
${bin_dir}gen-ref-colorchart \
    ${ref_illu} \
    ${ref_cmf} \
    output/reference.csv

# Create a preview of the reference patches
${bin_dir}gen-colorchart-image \
    output/reference.csv \
    output/reference.png


###############################################################################
# Fit matrix to correct colors
###############################################################################

${bin_dir}extract-matrix \
    output/reference.csv \
    output/patches.csv \
    output/matrix.csv


###############################################################################
# Color correction
###############################################################################

# Apply the matrix to each patch
${bin_dir}correct-patches \
    output/patches.csv \
    output/matrix.csv \
    output/corrected_patches.csv

# Create a preview of the corrected patches
${bin_dir}gen-colorchart-image \
    output/corrected_patches.csv \
    output/corrected_measured.png

# Correct the original image using the same matrix
${bin_dir}correct-image \
	${image_measurement} \
	output/matrix.csv \
	output/corrected_image.tiff
