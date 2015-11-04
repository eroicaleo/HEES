
% This script is based on house price example in neural network toolbox
% tutorial

% You will need to modify this part

initQ    = 100;  % Bank initial charge
superCap = 40;   % Bank capacitance

loThreshold = -inf; % the lower threshold for training power
upThreshold = inf; % the upper threshold for training power

% You should not need to modify the following part

close all;

energy_offset = (1/2)*initQ^2/superCap;
trnFile = sprintf('data/trn_10.txt.%04d', initQ);
tstFile = sprintf('data/tst_10.txt.%04d', initQ);
outFile = sprintf('data/nnetmodel%04d', initQ);

% Read training and testing data
A = dlmread(trnFile);
ix = (A(:, 1) < upThreshold) & (A(:, 1) > loThreshold);
inputs = A(ix, 1:2)';
targets = A(ix, 4)';
inputs(end, :) = inputs(end, :) - energy_offset;
targets = targets - energy_offset;

A = dlmread(tstFile);
ix = (A(:, 1) < upThreshold) & (A(:, 1) > loThreshold);
tstinputs = A(ix, 1:2)';
tsttargets = A(ix, 4)';
tstinputs(end, :) = tstinputs(end, :) - energy_offset;
tsttargets = tsttargets - energy_offset;

% Create a Fitting Network
hiddenLayerSize = 7;
net = fitnet(hiddenLayerSize);

% Set up Division of Data for Training, Validation, Testing
net.divideParam.trainRatio = 90/100;
net.divideParam.valRatio = 5/100;
net.divideParam.testRatio = 5/100;

% Train the Network
[net,tr] = train(net,inputs,targets);

% Test the Network
outputs = net(inputs);
errors = gsubtract(outputs,targets);
performance = perform(net,targets,outputs);

% View the Network
% view(net)

% Plots
% Uncomment these lines to enable various plots.
%figure, plotperform(tr)
%figure, plottrainstate(tr)
%figure, plotfit(targets,outputs)
%figure, plotregression(targets,outputs)
%figure, ploterrhist(errors)

tInd = tr.testInd;
tstOutputs = net(tstinputs);
tstErrPercent = tstOutputs ./ tsttargets;
plot(tstErrPercent);
figure;
plot((1:size(tstOutputs, 2)), tstOutputs, 'ro');
hold on
plot((1:size(tsttargets, 2)), tsttargets, 'bx');

% Extract the neural network parameters
% This is for older version of Matlab
% inputSettings = net.inputs{1}.processSettings{1, 2};
inputSettings = net.inputs{1}.processSettings{1};

% This is for older version of Matlab
% outputSettings = net.outputs{2}.processSettings{1, 2};
outputSettings = net.outputs{2}.processSettings{1};

if isfield(inputSettings, 'xmin') && isfield(outputSettings, 'xmin')

    inputMin = inputSettings.xmin;
    inputRange = inputSettings.xrange;

    outputMin = outputSettings.xmin;
    outputRange = outputSettings.xrange;

    IW = net.IW{1,1};
    LW = net.LW{2,1};

    b1 = net.b{1,1};
    b2 = net.b{2,1};

else

    inputMin   = min(inputs, [], 2);
    inputRange = range(inputs, 2);

    outputMin = min(targets);
    outputRange = range(targets);

    IW = zeros(hiddenLayerSize, 2);
    LW = zeros(1, hiddenLayerSize);

    b1 = zeros(hiddenLayerSize, 1);
    b2 = 0.0;
end


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Dump out the results for HEES simulator
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

fid = fopen(outFile, 'w');

fprintf(fid, 'NUM_OF_NEURONS: %d\n\n', hiddenLayerSize);
fprintf(fid, 'Input_dimension: %d\n', size(inputMin, 1));
fprintf(fid, 'Output_dimension: %d\n', size(outputMin, 1));
fprintf(fid, '\nInput_min:\n');
for i = 1:size(inputMin, 1)
    fprintf(fid, '%f\n', inputMin(i, 1));
end
fprintf(fid, '\nInput_range:\n');
for i = 1:size(inputMin, 1)
    fprintf(fid, '%f\n', inputRange(i, 1));
end
fprintf(fid, '\nOutput_min:\n');
for i = 1:size(outputMin, 1)
    fprintf(fid, '%f\n', outputMin(i, 1));
end
fprintf(fid, '\nOutput_range:\n');
for i = 1:size(outputMin, 1)
    fprintf(fid, '%f\n', outputRange(i, 1));
end
fprintf(fid, '\nIW:\n');
for i = 1:size(IW, 1)
    fprintf(fid, '%f %f\n', IW(i, :));
end
fprintf(fid, '\nb1:\n');
for i = 1:size(b1, 1)
    fprintf(fid, '%f\n', b1(i, :));
end
fprintf(fid, '\nLW\n');
for i = 1:size(LW, 2)
    fprintf(fid, '%f ', LW(1, i));
end
fprintf(fid, '\n\nb2:\n');
for i = 1:size(b2, 1)
    fprintf(fid, '%f\n', b2(i, :));
end

fclose(fid);
