
% Visualize the sweeping load power
% First, run the scripts/python/SweepLoadPower.py
% The results are dumpped out to SweepLoad.txt

energy   = load('SweepLoad.txt');
input    = energy(1, :);
energy   = energy(2:end, :);

plot(energy)
