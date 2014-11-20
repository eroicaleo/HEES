#!/usr/bin/env python3

'''
This script is to run multiple tests
'''

import datetime
import glob
import os
import os.path
import re
import shutil
import subprocess
import sys

num_of_tests = 20

if (not os.path.isfile('./energysys')) or (not os.path.isfile('./Scheduler')):
    sys.exit('Can not find binaries ./energysys or ./Scheduler')

#----------------------------------------------------------------------------------------------------
# create a folder with today's date
#----------------------------------------------------------------------------------------------------
today = datetime.datetime.today().strftime('%Y%m%d')
res_dir = 'results' + today
if not os.path.isdir(res_dir):
    os.mkdir(res_dir)

# Count how many sets of experiments have been done
curr_test_index = len(glob.glob(res_dir + '/TasksOrig.txt.*'))

summary_file = res_dir+'/summary.txt'
if os.path.isfile(summary_file):
    f_summ = open(summary_file, 'a')
else:
    f_summ = open(summary_file, 'w')
    print('%8s    '%('bspred'), file=f_summ, end='')
    print('%8s    '%('DPpred'), file=f_summ, end='')
    print('%8s    '%('bsreal'), file=f_summ, end='')
    print('%8s    '%('DPreal'), file=f_summ, end='')
    print(file=f_summ)

for i in range(num_of_tests):

    curr_test_index = "%04d" % (int(curr_test_index)+1)
    print(curr_test_index)
    #----------------------------------------------------------------------------------------------------
    # run the program and generate the trace
    #----------------------------------------------------------------------------------------------------

    # Run the scheduler to generate scheduling for both DP and baseline
    logname = 'log.' + curr_test_index
    f1 = open(logname, 'w')
    subprocess.call(['./Scheduler'], stdout=f1, stderr=f1)
    f1.close()

    energy_list = []
    pattern = re.compile('^Initial Energy: ([\d.]+)')
    for line in open(logname):
        m = re.match(pattern, line)
        if m:
            pattern = re.compile('^Final Energy: ([\d.]+) Finish Time: \d+')
            energy_list.append(m.group(1))
    shutil.move(logname, res_dir)

    shutil.move('TasksOrig.txt', res_dir+'/TasksOrig.txt.'+curr_test_index)

    # Run the simulator for baseline policy
    if not os.path.isfile('./TasksCATSFixed.txt'):
        sys.exit('Can not find binaries ./TasksCATSFixed.txt !')

    shutil.copyfile('TasksCATSFixed.txt', 'Tasks.txt')
    shutil.move('TasksCATSFixed.txt', res_dir+'/TasksCATSFixed.txt.'+curr_test_index)
    f1 = open('1', 'w')
    subprocess.call(['./energysys'], stdout=f1, stderr=f1)
    f1.close()
    for line in open('OverallProcess.txt'):
        pass
    base_energy = re.split(r'\s+', line.strip())[10]
    energy_list.append(base_energy)
    shutil.move('OverallProcess.txt', res_dir+'/OverallProcessCATS.txt.'+curr_test_index)

    # Run the simulator for DP policy
    if not os.path.isfile('./TasksDP.txt'):
        sys.exit('Can not find binaries ./TasksDP.txt !')

    shutil.copyfile('TasksDP.txt', 'Tasks.txt')
    shutil.move('TasksDP.txt', res_dir+'/TasksDP.txt.'+curr_test_index)
    f1 = open('1', 'w')
    subprocess.call(['./energysys'], stdout=f1, stderr=f1)
    f1.close()
    for line in open('OverallProcess.txt'):
        pass
    DP_energy = re.split(r'\s+', line.strip())[10]
    energy_list.append(DP_energy)
    shutil.move('OverallProcess.txt', res_dir+'/OverallProcessDP.txt.'+curr_test_index)

    #----------------------------------------------------------------------------------------------------
    # collect data and store the results as well as input trace, log file etc.
    #----------------------------------------------------------------------------------------------------

    initial_energy = energy_list.pop(0)
    print(['%.3f'%(float(e)-float(initial_energy)) for e in energy_list])
    [print('%8.3f'%(float(e)-float(initial_energy)), file=f_summ, end='    ') for e in energy_list]
    print(file=f_summ)

f_summ.close()
