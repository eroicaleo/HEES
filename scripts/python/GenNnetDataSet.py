#!/usr/bin/env python3

'''
This script generates neural network training/testing data
based on SweepLoad.txt produced by SweepLoadPower.py

1. Run it in the directory with the ./energysys
2. The output is in the nnet/data
'''

import glob
import os
import os.path
import re

predLen = 10
stepBetweenStartIndex = 5

def divideTrnTst(dataTrace):
    trnTrace = dataTrace[::2]
    tstTrace = dataTrace[1::2]

    if (len(dataTrace) % 2 == 0):
        trnTrace.append(dataTrace[-1])
        tstTrace.pop(-1)

    return trnTrace, tstTrace

def dumpDataset(energyTrace, trainOrTest, suffix):

    if (trainOrTest == 'train'):
        fileName = 'trn_' + '%2d'%(predLen) + '.txt'
    else:
        fileName = 'tst_' + '%2d'%(predLen) + '.txt'

    if suffix:
        fileName = fileName + "." + suffix

    if (not os.path.isdir('./nnet/data')):
        os.mkdir('./nnet/data')

    f = open('./nnet/data/' + fileName, 'w')
    for trace in energyTrace:
        for item in processOneInputPower(trace, predLen, stepBetweenStartIndex):
            for en in item:
                print('%8.3f'%(float(en)), end='    ', file=f)
            print(file=f)

    f.close()

def processOneInputPower(energyTrace, predLen=10, stepBetweenStartIndex=1):

    inputPower = energyTrace[0]
    energyTrace.pop(0)

    ix = list(range(len(energyTrace)))

    ixStart = ix[0::stepBetweenStartIndex]
    ixEnd   = ix[predLen::stepBetweenStartIndex]

    return ([inputPower, energyTrace[i], predLen, energyTrace[j]] for (i, j) in zip(ixStart, ixEnd))

def runIt():

    # Read the files
    for fname in glob.glob('SweepLoad.txt.*'):
        m = re.search(r'\.(\w+?)$', fname)
        suffix = m.group(1) if m else ''
        f = open(fname)
        energyTrace = []
        for line in f:
            line = line.strip()
            energyTrace.append(re.split(r'\s+', line))
        f.close()

        # Transpose the energyTrace
        energyTrace = list(map(list, zip(*energyTrace)))

        # Divide training and testing data
        energyTrnTrace, energyTstTrace = divideTrnTst(energyTrace)

        # Dump the data set
        # print(energyTrnTrace)
        dumpDataset(energyTrnTrace, 'train', suffix)
        dumpDataset(energyTstTrace, 'test', suffix)

    return;

if __name__ == '__main__':
    runIt()
