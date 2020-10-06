#!/usr/bin/env python3

from PIL import Image
import numpy as np
from color import convert
import argparse

def extract_patches(im: Image, bounding_boxes: list) -> list:
    imarray_measured = np.array(im)

    macbeth_patches_measured = []

    # Extract each color patch
    for bbox in bounding_boxes:
        top_left = bbox[0]
        bottom_right = bbox[1]
        
        acc = np.zeros(3)

        # Average the patch over the box area
        for x in range(top_left[0], bottom_right[0]):
            for y in range(top_left[1], bottom_right[1]):
                acc += imarray_measured[y, x]

        acc /= np.ones(3)*255*((bottom_right[0] - top_left[0])*(bottom_right[1] - top_left[1]))

        macbeth_patches_measured.append([convert.from_sRGB(c) for c in acc])

    return macbeth_patches_measured


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Utility to extract averaged values from Macbeth captured image')
    parser.add_argument('-i', '--image' , type=str, required=True, help='Image file of Macbeth colorchart')
    parser.add_argument('-a', '--areas' , type=str, required=True, help='File containing bounding boxes of each patch')
    parser.add_argument('-o', '--output', type=str, required=True, help='CSV output file to write averaged values')

    args = parser.parse_args()

    im_measured = Image.open(args.image)
    
    areas = []
    with open(args.areas) as f:
        for l in f:
            v = [int(v) for v in l.split(',')]
            areas.append([(v[0], v[1]), (v[2], v[3])])

    macbeth_patches_measured = extract_patches(im_measured, areas)

    # Save color patches to file
    with open(args.output, 'w') as f:
        for patch in macbeth_patches_measured:
            f.write(str(patch[0]) + ', ' + str(patch[1]) + ', ' + str(patch[2]) + '\n')
