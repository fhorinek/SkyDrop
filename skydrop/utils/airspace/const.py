
from math import sqrt

DATA_LEVELS = 5
DATA_LEVEL_SIZE = 3

DATA_INDEX_SIZE = 64

MIN_RES = 6
                     
OFFSET_BASE  = 0.5                           
OFFSET_MUL_0 = OFFSET_BASE / 64
OFFSET_MUL_1 = (OFFSET_MUL_0 * MIN_RES) / 64
OFFSET_MUL_2 = (OFFSET_MUL_1 * MIN_RES) / 64

AIRSPACE_BORDER = OFFSET_BASE

HGT_COORD_MUL = 10000000

MAX_ALTITUDE = 0x7FFF

LAT_MINIMUM = -60

CLASS_DICT = {
    "A":            0,
    "B":            1,
    "C":            2,
    "D":            3,
    "E":            4,
    "F":            5,
    "G":            6,
    "CTR":          7,
    "CTA":          8,
    "TMA":          9,
    "DANGER":       10,
    "RESTRICTED":   11,
    "PROHIBITED":   12,
    "TMZ":          13,
    "RMZ":          14,
    "FIR":          15,
    "UIR":          16,
    "WAVE":         17,
    "GLIDING":      18,
    "OTH":          19,
}        

CLASS_FILTER = {
    "A":            True,   #Class A
    "B":            True,   #Class B
    "C":            True,   #Class C
    "D":            True,   #Class D
    "E":            True,   #Class E
    "F":            True,   #Class F
    "G":            True,   #Class G
    "CTR":          True,   #Control zone
    "CTA":          True,   #Control zone
    "TMA":          True,   #Terminal control area
    "DANGER":       True,   #Danger
    "PROHIBITED":   True,   #Prohibited
    "RESTRICTED":   True,   #Restricted
    "TMZ":          True,   #Transponder Mandatory Zone
    "RMZ":          True,   #Radio mandatory zone
    "FIR":          True,   #Flight information region
    "UIR":          False,  #Upper information region Above FIR
    "WAVE":         False,  #Wave window
    "GLIDING":      False,  #Gliding area
    "OTH":          False,  #Other
}        

CLASS_SCORE = {
    "A":            1.0,  #Class A
    "B":            1.0,  #Class B
    "C":            1.0,  #Class C
    "D":            1.0,  #Class D
    "E":            1.0,  #Class E
    "F":            1.0,  #Class F
    "G":            1.0,  #Class G
    "CTR":          1.0,  #Control zone
    "CTA":          1.0,  #Control zone
    "TMA":          1.0,  #Terminal control area
    "DANGER":       1.0,  #Danger
    "PROHIBITED":   1.0,  #Prohibited
    "RESTRICTED":   0.5,  #Restricted
    "TMZ":          0.75, #Transponder Mandatory Zone
    "RMZ":          0.75, #Radio mandatory zone
    "FIR":          0.5,  #Flight information region
    "UIR":          0.4,  #Upper information region Above FIR
    "WAVE":         0.2,  #Wave window
    "GLIDING":      0.2,  #Gliding area
    "OTH":          0.1   #Other
}


