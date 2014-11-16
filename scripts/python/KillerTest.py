#!/usr/bin/env python3

import math
import re
import subprocess

initCharge = 1400.0 # (40, 80, 100, 200, 1600)
current    = 2.0 # (0.6, 1.0, 2.0, 2.2, ..., 3.0)
dvfs       = 0.0 # (0.0, 1.0, 2.0)
taskLength = 200.0

##----------------------------------------------------------------------------------------------------
## Do not change the following
##----------------------------------------------------------------------------------------------------
initEnergy = 0.5 * 40 * (initCharge/40.0)**2

def generateWorkload(current):
    volLevel = [d/10.0 for d in range(8, 13)]
    res = []
    for v in volLevel:
        l = math.ceil(taskLength/v)
        tmp = [(v, current*(v**dvfs), l)]
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
    initChargeList = [40.0, 80.0]
    initChargeList.extend(list(range(100, 1700, 100)))
    dvfsList = [0.0, 1.0, 2.0]
    currentList = [0.1*d for d in range(6, 32, 2)]
    for dvfs in dvfsList:
        for current in currentList:
            for initCharge in initChargeList:
                runIt()
