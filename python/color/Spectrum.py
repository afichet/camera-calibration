import math

def mix(a, b, v):
    return (1 - v)*a + v*b

def place(v_min, v_max, v):
    v = min(v_max, max(v_min, v))
    return (v - v_min) / (v_max - v_min)

class Spectrum:
    def __init__(self, wavelengths: list, values: list):
        # Sanity check
        if len(wavelengths) != len(values):
            raise Exception("Not the same number of wavelengths and values")
        
        # Todo: sort by wavelength
        self.start = int(min(wavelengths))
        self.end   = int(max(wavelengths))
        
        n_vals = self.end - self.start + 1
        self.spectrum = n_vals * [0]

        j = 1
        
        self.spectrum[0] = values[0]
        
        for i in range(1, n_vals):
            curr_wavelength = self.start + i

            if curr_wavelength > wavelengths[j]:
                j +=1
                assert(j < len(wavelengths))
            
            interp = place(wavelengths[j-1], wavelengths[j], curr_wavelength)
            self.spectrum[i] = mix(values[j-1], values[j], interp)
            
    
    def idx_for_wavelength(self, wavelength: float) -> int:
        return int(wavelength - self.start)

    
    def value(self, wavelength: float) -> float:
        if wavelength < self.start or wavelength > self.end:
            return 0.0
        
        idx = self.idx_for_wavelength(wavelength)
        return self.spectrum[idx]
    

    def integrate(self, start: float = 380, end: float = 800) -> float:
        a = 0
        for wl in range(start, end):
            a += (self.value(wl) + self.value(wl+1))/2
        return a
    

    def mul(self, other):
        start_wl, end_wl = max(self.start, other.start), min(self.end, other.end)
        wavelength = [wl for wl in range(start_wl, end_wl+1)]
        values = [self.value(wl)*other.value(wl) for wl in wavelength]
        return Spectrum(wavelength, values)
    

    def mul_int(self, other, start: float = 380, end: float = 800) -> float:
        a = 0
        for wl in range(start, end):
            a += (self.value(wl)*other.value(wl) + self.value(wl+1)*other.value(wl))/2
        return a
    
        return (self.mul(other)).integrate(start, end)