#!/usr/bin/env python
import sys
from pcbnew import *

filename=sys.argv[1]

pcb = LoadBoard(filename)

for module in pcb.GetModules():    
    print "* Module: %s"%module.GetReference()
    ref = module.GetReferenceObj()
    if module.GetReference()=="MHOLE":
        continue
    ref.SetVisible(True)   # set Reference as Visible
    ref.SetSize(wxSizeMM(0.4572,0.7112))
    ref.SetThickness(ToMM(0.127))
    
    
pcb.Save("mod_"+filename)	