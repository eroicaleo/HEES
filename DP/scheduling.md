TODO list
=========

1. Process the tasks and generate an array of tasks.
   It is there. Just use the BuildTaskVoltageTableVectorFromFile function. 

2. Process the solar power and generate an array of solar power.
   Finished.	

3. Pick two tasks, generate a series of charging phase.

4. Make predictions based on the charging phases.
   Finished.

HOWTO
=====

1. Modify the VariablePowerSource.txt to change the solar profile.
  You can use the current one in your first trial.
  The first number is the solar power and the second number is length (in second).
  So the following trace means in the first 250 seconds the solar power is
  4.0w and the next 200 seconds, the solar power is 4.4w

  ```
  4.0 250
  4.4 200
  ```

2. Modify the TasksSolar.txt to change the task set.
  You can use the current one in your first trial.
  The 1st number is task length, the 2nd number is task power and the 3rd number
  is task energy, assuming task is running at 1.0 volt.

  ```
  100 1.8 180
  250 1.6 400
  100 2.0 200
  ```

3. run the `./swap` command. It generates 4 files

  ```
  TasksSCHEDForDP.txt       // scheduled task set for DFVS  
  TasksSCHEDForDP.txt.init  // Initial task without scheduling
  TasksSCHEDForEES.txt      // scheduled task set for HEES simulator
  TasksSCHEDForEES.txt.init // Initial task set for HEES simulator
  ```

  Then do the following:

  ```
  cp VariablePowerSource.txt ../
  cp TasksSCHEDForEES.txt.init ../Tasks.txt
  cd ../
  ./energysys > 1 # record the final energy in OverallProcess.txt, this is the one for non scheduled task set
  cd DP/
  cp TasksSCHEDForEES.txt ../Tasks.txt
  cd ../
  ./energysys > 1 # record the final energy in OverallProcess.txt, this is the one for scheduled task set

  # Compare the energy

  ```

4. run the ./all command. It takes `TasksSCHEDForDP.txt` as input and
   generates 1 file

   ```
   TasksDPForEES.txt
   ```

   Then do the following:

   ```
   cp TasksDPForEES.txt ../Tasks.txt
   ./energysys > 1 # record the final energy in OverallProcess.txt, this is the one with scheduling+DVFS
   ```

