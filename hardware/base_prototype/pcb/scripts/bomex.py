#!/usr/bin/env python
import sys
from pcbnew import *

filename=sys.argv[1]
filename_out = sys.argv[2]

SEPARATOR=";"

pcb = LoadBoard(filename)

dict = {}

for module in pcb.GetModules():    
    designator = module.GetReference()
    value = module.GetValue()
    footprint = module.GetLibRef()
    description = module.GetDescription()
  
    k = "%s%s%s%s%s"%(value,SEPARATOR,footprint,SEPARATOR,description)
    
    if dict.has_key(k):
        dict[k].append(module)
    else:
        dict[k]=[module]

f = open(filename_out,"w")
f.write("value;footprint;description;references;items\n")    
for k in dict.keys():
    elements = dict[k]
    n = len(elements)
    line = k+SEPARATOR
    for element in elements:
        line += element.GetReference() + " "
    line += SEPARATOR + str(n)
    f.write(line+"\n")
    
f.close()    
    
    