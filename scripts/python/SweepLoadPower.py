#!/usr/bin/env python3

'''
This script sweeps the load power and
generate supercapacitor energy curve.

1. Run it in the directory with the ./energysys
2. The output is in the SweepLoad.txt
3. Use matlab/octave PlotSweepLoad.m to plot the data
'''

import os
import os.path
import re
import subprocess
import sys

def processLogFile():
  f = open('OverallProcess.txt', 'r')
  # Skip the header
  f.readline()
  res = [[re.split(r'\s+', line)[i] for i in [8, 11]] for line in f]
  energy = [i[1] for i in res]
  inputPower = res[0][0]
  return inputPower, energy

##----------------------------------------------------------------------------------------------------
## Sanity check
##----------------------------------------------------------------------------------------------------
if not os.path.isfile('energysys'):
  sys.exit('Can not find energysys binary in current dir!')

##----------------------------------------------------------------------------------------------------
## Generate load power sequence
##----------------------------------------------------------------------------------------------------
initalPower = 0.0
step        = 0.2
tasklen     = 4000
NumofLoad   = 10
InitialCharge = 1300
LoadPower = [initalPower+step*x for x in range(NumofLoad)]

energyList = []

for power in LoadPower:
  f = open('Tasks.txt', 'w')
  print('1.0 %.2f %s' % (power, tasklen), file=f)
  f.close()
  print('Run the load power ', power)
  # Run the simulator
  f1 = open('1', 'w')
  subprocess.call(['./energysys', '--supcap_init_charge=%.2f' % InitialCharge], stdout=f1, stderr=f1)
  f1.close()
  # Collect the power number
  inputPower, energy = processLogFile()
  energyList.append([inputPower] + energy)
  # Dump out the results
  f = open('SweepLoad.txt.%04d'%InitialCharge, 'w')
  for i in range(len(energyList[0])):
    for j in range(len(energyList)):
      print('%8.3f'%(float(energyList[j][i])), file=f, end=' '*4)
    print(file=f)
  f.close()
