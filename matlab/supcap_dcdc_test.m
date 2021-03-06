% This matlab script is used to compute the output voltage and current
% when the DC-DC converter is connected to the supercapacitor
addpath('./matlab');

global V_out I_out Racc Cacc Qacc;

% Read the input from C++ simulator
input = load('dcdc_supcap_input.txt'); 
V_out = input(1);
I_out = input(2);
Racc = input(3);
Cacc = input(4);
Qacc = input(5);

% Guess a solution as initial point
x0 = [10; 0];
options=optimset('Display','off');
% Assume first it is in buck mode
[x,fval] = fsolve(@supcap_to_dcdc_model_buck, x0, options);
V_in = x(1);
I_in = x(2);

if (V_out > V_in)
    % If it actually is not in buck mode, recompute it using boost mode
    [x,fval] = fsolve(@supcap_to_dcdc_model_boost, x, options);
    V_in = x(1);
    I_in = x(2);
    if (V_out < V_in)
        x = [-1.0; -1.0; -1.0];
        save('dcdc_supcap_output.txt', '-ascii', 'x');
        error('Neither buck mode or boost mode!');
    end
end

power = dcdc_model_compute_power(V_in, I_in, V_out, I_out);
x = [x; power];

save('dcdc_supcap_output.txt', '-ascii', 'x');
