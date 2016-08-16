#!/usr/bin/env python3

'''
This script generates bar graph data
in DAC paper for 10/20/30 min tasks
'''

import os
import os.path
import re
import subprocess
import sys
import shutil
import math

voltage = 1.0
length = [60, 120]

task_power = [1 + p/10.0 for p in range(2)]
print(task_power)

for l in length:
	for p1 in task_power:
		for p2 in task_power:
			fname = "Tasks_%.1f_%.1f_%04d.txt" % (p1, p2, l)
			f = open(fname, 'w')
			print("%.1f %.1f %d" % (voltage, p1, l), file = f)
			print("%.1f %.1f %d" % (voltage, p2, l), file = f)
			f.close()
			shutil.copy(fname, 'Tasks.txt')
			f3 = open('1', 'w')
			subprocess.call(['./energysys', 
								'--power_source=%s' % ('constant_power'), 
								'--constant_power_value=%f' % (2.0),
								'--dc_load_is_ideal=1',
							], stdout=f3, stderr=f3)
			f3.close()
			oname = "OverallProcess_%.1f_%.1f_%04d.txt" % (p1, p2, l)
			shutil.copy('OverallProcess.txt', oname)
