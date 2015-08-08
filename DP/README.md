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
	* `./solar > 1.txt`
	* results are in the 1.txt. Expected results: for the first 250 sec, the solar power is 4.0.
	The next 200 sec, the solar power is 4.4.
