#!/usr/bin/env python3

import math
import re
import subprocess

initCharge = 800.0
current    = 3.0
taskLength = 100.0

##----------------------------------------------------------------------------------------------------
## Do not change the following
##----------------------------------------------------------------------------------------------------
initEnergy = 0.5 * 40 * (initCharge/40.0)**2

def generateWorkload(current):
    volLevel = [d/10.0 for d in range(8, 13)]
    res = []
    for v in volLevel:
        l = math.ceil(taskLength/v)
        tmp = [(v, current*(v**0), l)]
        rest = math.ceil(taskLength/min(volLevel)) - l
        if rest:
            tmp.append((1.0, 0.0, rest))
        res.append(tmp)
    return res

def runIt():
    for tasklist in generateWorkload(current):
        with open('Tasks.txt', 'w') as f:
            for task in tasklist:
                print('%.2f %.3f %d' % (task), file=f)
        with open('1') as f:
            subprocess.call(['./energysys', '--supcap_init_charge=%.2f' % initCharge], stdout=f, stderr=f)
        with open('OverallProcess.txt') as f:
            for line in f: pass
        line = line.strip()
        finalEnergy = float(re.split(r'\s+', line)[10]) - initEnergy
        print("%.3f" % finalEnergy)

if __name__ == '__main__':
    runIt()
