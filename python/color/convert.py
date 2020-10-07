from color.Spectrum import Spectrum

def XYZ_to_RGB(XYZ: list) -> list:
    R =  3.2404542*XYZ[0] - 1.5371385*XYZ[1] - 0.4985314*XYZ[2]
    G = -0.9692660*XYZ[0] + 1.8760108*XYZ[1] + 0.0415560*XYZ[2]
    B =  0.0556434*XYZ[0] - 0.2040259*XYZ[1] + 1.0572252*XYZ[2]
    
    return R, G, B


def to_sRGB(C:float) -> float:
    if C <= 0:
        return 0
    elif C >= 1:
        return 1
    elif C <= 0.0031308:
        return 12.92 * C
    return 1.055 * C**0.41666 - 0.055


def from_sRGB(C:float) -> float:
    if C <= 0.04045:
        return C / 12.92
    return ((C + 0.055) / 1.055)**2.4


def spectrum_emission_to_XYZ(
    e_spectrum: Spectrum, xyz_bar: list, 
    start:float = 380, end:float = 800) -> list:

    XYZ = [e_spectrum.mul_int(cmf, start, end) for cmf in xyz_bar] 
    
    return XYZ


def spectrum_reflectance_to_XYZ(
    r_spectrum: Spectrum, illuminant: Spectrum, xyz_bar: list, 
    start:int = 380, end:float = 800) -> list:

    N = illuminant.mul_int(xyz_bar[1], start, end)
    refl_illu = r_spectrum.mul(illuminant)
    XYZ_refl = spectrum_emission_to_XYZ(refl_illu, xyz_bar, start, end)
    
    return [r/N for r in XYZ_refl]


def spectrum_emission_to_RGB(
    e_spectrum: Spectrum, xyz_bar: list, 
    start:int = 380, end:float = 800) -> list:

    XYZ = spectrum_emission_to_XYZ(e_spectrum, xyz_bar, start, end)
    return XYZ_to_RGB(XYZ)


def spectrum_reflectance_to_RGB(
    r_spectrum: Spectrum, illuminant: Spectrum, xyz_bar: list,
    start:int = 380, end:float = 800) -> list:

    XYZ = spectrum_reflectance_to_XYZ(r_spectrum, illuminant, xyz_bar, start, end)
    return XYZ_to_RGB(XYZ)


from colormath.color_objects import XYZColor, LabColor
from colormath.color_conversions import convert_color
from colormath.color_diff import delta_e_cie2000

def delta_e_from_XYZ(XYZ1, XYZ2):
    c1 = XYZColor(XYZ1[0], XYZ1[1], XYZ1[2])
    c2 = XYZColor(XYZ2[0], XYZ2[1], XYZ2[2])

    c1_lab = convert_color(c1, LabColor)
    c2_lab = convert_color(c2, LabColor)
    
    #L1, a1, b1 = to_Lab(XYZ1, illuminant, xyz_bar)    
    #L2, a2, b2 = to_Lab(XYZ2, illuminant, xyz_bar)
    
    #c1_lab = LabColor(L1, a1, b1)
    #c2_lab = LabColor(L2, a2, b2)

    return delta_e_cie2000(c1_lab, c2_lab)