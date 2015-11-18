#!/usr/bin/env python
import sys
from pcbnew import *

# split a string by the comma, and clean leading/trailing spaces
def splitAndClean(s):
    lst_a = s.split(",")
    lst = []
    for item in lst_a:
        lst.append(item.strip())
    return lst

def getModules(pcb,s):
    lst = splitAndClean(s)
    modules = []
    for item_ref in lst:
        modules.append(pcb.FindModuleByReference(item_ref))
    return modules
    

vector = wxPointMM(0,25.0)

group1 = "C28,C27,D12,R23,C32,R22,D9,Q5,D10,L4,C30,C29,P2,U4,C26,D11,R21,C16,C15,Q6,R20,C31"
group2 = "C19,C18,D8 ,R17,C23,R16,D5,Q3,D6 ,L3,C21,C20,P3,U3,C17,D7 ,R15,C25,C24,Q4,R14,C22"
group3 = "C10,C9 ,D4 ,R11,C14,R10,D1,Q1,D2 ,L2,C12,C11,P4,U2,C8 ,D3 ,R9 ,C7 ,C6 ,Q2,R8 ,C13"


filename=sys.argv[1]
pcb = LoadBoard(filename)

director_group = getModules(pcb,group1)
moved_groups = [getModules(pcb,group2),getModules(pcb,group3)]

i = 0

for i in range(len(director_group)):
    director_element = director_group[i]
    pos = director_element.GetPosition()
    angle = director_element.GetOrientation()
    r_pos = director_element.GetReferenceObj().GetPos0()
    r_angle = director_element.GetReferenceObj().GetOrientation()
        
    # move our group elements (vector*n) and copy relative references 
    for moved_group in moved_groups:
        pos = pos + vector
        moved_group[i].SetPosition(pos)
        moved_group[i].SetOrientation(angle)
        moved_group[i].GetReferenceObj().SetPos0(r_pos)
        moved_group[i].GetReferenceObj().SetOrientation(r_angle)
            
            
pcb.Save("mod_"+filename)	
