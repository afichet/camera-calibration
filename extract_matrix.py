#!/usr/bin/env python3

import argparse
import csv
from color.Spectrum import Spectrum
from color import load, convert
from scipy.optimize import least_squares
import numpy as np

def p_cost(x, *args, **kwargs):
    e = []
    patches_ref = args[0]
    patches_meas = args[1]
    
    for xyz_ref, xyz_meas in zip(patches_ref, patches_meas):
        xyz_corrected = np.matmul(x.reshape(3, 3), xyz_meas)
        e.append(convert.delta_e_from_XYZ(xyz_ref, xyz_corrected))
        
    return e


def calibration(
    macbeth_reference_spectra: list, 
    macbeth_camera_colors: list,
    illuminant_spd: Spectrum,
    cmf: list):

    macbeth_reference_colors = [convert.spectrum_reflectance_to_XYZ(s, illuminant_spd, cmf) for s in macbeth_reference_spectra]

    res = least_squares(
        p_cost, 
        np.array([1, 0, 0, 0, 1, 0, 0, 0, 1]),
        xtol=1e-15, ftol=1e-15,
        args=[macbeth_reference_colors, macbeth_camera_colors]
    )

    return res.x.reshape(3, 3)

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Utility to compute conversion matrix from a capture set of Macbeth patches')
    parser.add_argument('-r', '--reference' , type=str, required=True, help='File containing Macbeth reference spectra')
    parser.add_argument('-c', '--camera'    , type=str, required=True, help='File containing Macbeth camera colours')
    parser.add_argument('-i', '--illuminant', type=str, required=True, help='File containing illuminant spectrum')
    parser.add_argument(      '--cmf'       , type=str, required=True, help='File containing color matching functions')
    parser.add_argument('-o', '--output'    , type=str, required=True, help='Output file to write resulting matrix')

    args = parser.parse_args()

    illuminant_spd = load.spd(args.illuminant)
    cmf = load.cmf(args.cmf)
    macbeth_reference_spectra = load.macbeth_spectra(args.reference)
    macbeth_camera_colors = load.macbeth_colors(args.camera)

    mat = calibration(macbeth_reference_spectra, macbeth_camera_colors, illuminant_spd, cmf)

    with open(args.output, 'w') as f:
        for l in mat:
            for c in l:
                f.write(str(c) + ', ')
            f.write('\n')