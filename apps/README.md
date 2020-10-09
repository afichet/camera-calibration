# Applications for camera calibration

This folder contains all the applications needed for camera calibration.

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