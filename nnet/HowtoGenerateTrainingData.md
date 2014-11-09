How to generate training/testing data and nnet model
=============================

We set the solor power at 4.0w, and we sweep
the task current from 0.0w to 3.0w with a step of 0.2w .
The resulting charging power to the EES bank is from 3.936w to 0.135w.

We use the scripts under scripts/python to generate the dataset.
The final data set is under "./nnet/data".

We plan to make 5 sets of training data, with a step of
100, 40, 20, 10 and all less than 5.
We plan to generate more data when the supercapacitor energy is small.
Because the energy changing slope is more steep.

However, currently we only have training data with step of 10.
