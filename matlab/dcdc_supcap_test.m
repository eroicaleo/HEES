% This matlab script is used to compute the output voltage and current
% when the DC-DC converter is connected to the supercapacitor
addpath('./matlab');

global V_in I_in Racc Cacc Qacc;

% Read the input from C++ simulator
input = load('dcdc_supcap_input.txt'); 
V_in = input(1);
I_in = input(2);
Racc = input(3);
Cacc = input(4);
Qacc = input(5);
Voc = Qacc / Cacc;

% Guess a solution as initial point
x0 = [0.01; 0];
options=optimset('Display','off');
% Assume first it is in buck mode
[x,fval] = fsolve(@dcdc_model_to_supcap_buck, x0, options);
V_out = x(1);
I_out = x(2);

if ((V_out > V_in) || (abs(fval(1, 1))))
    % If it actually is not in buck mode, recompute it using boost mode
    [x,fval] = fsolve(@dcdc_model_to_supcap_boost, x, options);
    V_out = x(1);
    I_out = x(2);
    if (V_out < V_in)
		x = [Voc; 0.0];
        V_out = Voc; 
        I_out = 0.0;
        warning('Neither buck mode or boost mode!');
    end
end

power = dcdc_model_compute_power(V_in, I_in, V_out, I_out);
x = [x; power];

save('dcdc_supcap_output.txt', '-ascii', 'x');
