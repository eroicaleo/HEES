How to use matlab to train neural network model
===============================================

## 1. Go to directory

'''shell
cd ~/training/matlab/nnet
'''

## 2. Start matlab, typing the following in the command line

'''shell
submat
'''

## 3. open script
Once matlab is open, double click 'nnet\_approximation.m' to open the script
Only thing needs to be changed is the "initQ" variable, depending on which
initial bank charge you want to set.

Set break point at line 47 and line 75. Run the matlab code.
Check figure 1 for prediction errors on testing data. Ideally, it should be close to 1.
We can accept the value range from 0.97 to 1.03, i.e. 3% error.

If you are satisfy the error, than let the program run through. Otherwise, stop the program
and start over again.

The results will be in the data/nnetmodel\* file

If the error is constantly bigger we expect, we might need to change the training data range.
To limit the training data range, change the loThreshold and upThreshold.
For example, if we set the loThreshold = 1 and upThreshold = inf, we limit the input power in
the training set above 1.0w.
In this case, you need also train another model for input power less than 1.0w.
You need to rename the output file name after model training is complete.
