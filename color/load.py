import csv
from color.Spectrum import Spectrum

# Loads SPDs
def spd_raw(filename: str):
    wavelength_spd = []
    spd = []
    with open(filename) as csvD65:
        csvReader = csv.reader(csvD65)

        for row in csvReader:
            wavelength_spd.append(float(row[0]))
            spd.append(float(row[1]))

    return (wavelength_spd, spd)


def spd(filename: str):
    wavelength_spd, spd = spd_raw(filename)
    
    return Spectrum(wavelength_spd, spd)


# Loads CMFs
def cmf_raw(filename: str):
    wavelength_xyz = []
    xyz = [[], [], []]
    with open(filename) as csvXYZ:
        csvReader = csv.reader(csvXYZ)

        for row in csvReader:
            wavelength_xyz.append(float(row[0]))

            for i in range(3):
                xyz[i].append(float(row[i+1]))
            
    return (wavelength_xyz, xyz)


def cmf(filename: str):
    wavelength_xyz, xyz = cmf_raw(filename)

    return [Spectrum(wavelength_xyz, xyz[i]) for i in range(3)]


# Macbeth

def macbeth_spectra(filename: str):
    wavelengths = []
    refl = []

    with open(filename) as csvSpectra:
        csvReader = csv.reader(csvSpectra)
        i = 0
        
        for row in csvReader:
            if i == 0:
                wavelengths += [float(r) for r in row]
            else:
                refl.append([float(r) for r in row])
            i += 1

    macbeth_patches = []

    for r in refl:
        macbeth_patches.append(Spectrum(wavelengths, r))

    return macbeth_patches


def macbeth_colors(filename: str):
    macbeth_patches = []

    with open(filename) as csvXYZ:
        csvReader = csv.reader(csvXYZ)
        
        for row in csvReader:
            macbeth_patches.append([float(r) for r in row])

    return macbeth_patches
