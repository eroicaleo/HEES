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
	* ./sched > 1.txt
	* resutls are in the 1.txt
