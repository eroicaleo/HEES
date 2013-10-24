% This matlab script is used to compute the output voltage and current
% when the DC-DC converter is connected to the Lion battery

addpath('./matlab');

global V_in I_in V_SOC;

% Read the input from C++ simulator
input = load('dcdc_battery_input.txt'); 
V_in = input(1);
I_in = input(2);
V_SOC = input(3);

% Guess a solution as initial point
x0 = [5; 1];
options=optimset('Display','off');
% Assume first it is in buck mode
[x,fval] = fsolve(@dcdc_model_to_battery_buck, x0, options);
V_out = x(1);
I_out = x(2);

if (V_out > V_in)
    % If it actually is not in buck mode, recompute it using boost mode
    [x,fval] = fsolve(@dcdc_model_to_battery_boost, x, options);
    V_out = x(1);
    I_out = x(2);
    if (V_out < V_in)
        x = [-1.0; -1.0; -1.0];
        save('dcdc_output_battery.txt', '-ascii', 'x');
        error('Neither buck mode or boost mode!');
    end
end

power = dcdc_model_compute_power(V_in, I_in, V_out, I_out);
x = [x; power];

save('dcdc_output.txt', '-ascii', 'x');
