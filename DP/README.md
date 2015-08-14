README for Dynamic Programming Part
===================================

Test Suite
----------

### 03. testSched.cpp

0. Purpose
This test is intended to test the scheduling part

1. How to build the test

    ```shell
    make sched
    ```

2. How to run it
    * generate a task file: "TaskSched.txt", like:
	```python
	# taskLength taskPower taskEnergy
	100 1.0 100
	100 1.5 150
	100 2.0 200
	```
	* `./sched > 1.txt`
	* results are in the 1.txt

### 04. testVariableSolarPower.cpp

0. Purpose
This test is intended to test the solar power can be change in the dp table or not

0. Go to project/DP directory

1. How to build the test

    ```shell
    make solar
    ```

2. How to run it
    * generate a task file: "TasksSolar.txt", like:
	```python
	# taskLength taskPower taskEnergy
	100 1.0 100
	100 1.5 150
	100 2.0 200
	```
	* generate a "VariablePowerSource.txt", like:
	```python
	# solarPower Length
	4.0 250
	4.4 200
	```
	* In the hees.cfg, change the power_source option to "variable_power"
	```python
	power_source = variable_power
	```
	* In the hees.cfg, change the `supcap_init_charge` to 100 or 600, you can first try 600
	* `./solar > 1.txt`
	* results are in the 1.txt. Expected results: for the first 250 sec, the solar power is 4.0.
	The next 200 sec, the solar power is 4.4.

3. Compare the prediction accuracy
	* Now go up one level, Change the "Tasks.txt" accordingly, the "Tasks.txt" format and "TasksSolar.txt"
	format is different, which is annoying. I will change in the future.
	The following file "Tasks.txt" corresponds to the "TasksSolar.txt" above.
	```python
	# taskVoltage taskCurrent taskLength
	1.0 1.0 100
	1.0 1.5 150
	1.0 2.0 200
	```
	* Change the "hees.cfg" accordingly, `power_source`, `supcap_init_charge`
	* `./energysys > 1`
	* Now compare the energy results between "1.txt" and "OverallProcess.txt"
