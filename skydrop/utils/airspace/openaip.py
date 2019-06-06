from functools import lru_cache
from lxml import etree
import re
import xml.etree.ElementTree as ET
from const import *
import string

class AipAltitude:
    def __init__(self, raw):
        reference = raw.get('REFERENCE')
        
        if reference == "GND":
            self.agl = True
        elif reference in ["MSL", "STD"]:
            self.agl = False
        else:
            raise Exception("No altitude reference!")
            
        unit = raw.find('ALT').get('UNIT')
        self.value = int(float(raw.find('ALT').text))
        
        if unit == "FL":
            self.value *= 100

    def __repr__(self):
        return '{}ft {}'.format(self.value, "AGL" if self.agl else "MSL")

#    def get(self):
#        return self.value, self.agl
        
class AipAirspace:
    name = None
    category = None
    top = None
    bottom = None
    invalid = True

    def __init__(self, raw):
        try:
            self._raw = raw
            self.category = self._raw.get('CATEGORY')
            self.name = ''.join(filter(lambda x:x in string.printable, self._raw_value('NAME')))
            self.country = self._raw_value('COUNTRY')
            
            geom = self._raw.find('GEOMETRY/POLYGON')
            self.coordinates = [tuple(map(float, lonlat.strip().split())) for lonlat in geom.text.split(',')]
            
            self.top = AipAltitude(self._raw.find('ALTLIMIT_TOP'))
            self.bottom = AipAltitude(self._raw.find('ALTLIMIT_BOTTOM'))

        except Exception as e:
            print(self, "Parsing error", e)
            
            return
        
        if self.top.value == self.bottom.value:
            print(self, "Top and bottom are the same! Droping")
            return
        
        self.invalid = False

        if self.category not in CLASS_FILTER:
            print(self, "Unknown category '%s'! Setting to OTH" % self.category)
            self.category = "OTH"
        

        if self.top.value < self.bottom.value:
            print(self, "Top is smaller than bottom! Switching")
            c = self.top
            self.top = self.bottom
            self.bottom = c
    
    def _raw_value(self, key):
        return self._raw.find(key).text
    
    def __repr__(self):
        return '{} [{}] {} ({} - {}) {}'.format(self.name, self.category, self.country, self.bottom, self.top, "INVAILID" if self.invalid else "")

    

def load_file(filepath):

    f = open(filepath, "r")

    regex = re.compile(r"&(?!amp;|lt;|gt;)")
    data = regex.sub("&amp;", f.read())
    
    f.close()

    root = ET.fromstring(data)
    airspaces = root.find('AIRSPACES')
    ars = []
    for el in airspaces:
        ars.append(AipAirspace(el))
        
    return ars
    

