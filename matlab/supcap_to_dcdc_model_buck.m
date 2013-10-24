function F = supcap_to_dcdc_model_buck(x)

V_in = x(1);
I_in = x(2);
% syms I_out;
% syms V_out;
% syms V_in I_in;
global V_out I_out Racc Cacc Qacc;

Lf = 6 * 10^-6; % 6 uH
fs = 500 * 10^3; % 500 kHz

R_L = 39 * 10^-3; % 39 mOhm
R_sw1 = 25 * 10^-3; % 25 mOhm
R_sw2 = 25 * 10^-3;
R_sw3 = 25 * 10^-3;
R_sw4 = 25 * 10^-3;

R_C = 100 * 10^-3; % 100 mOhm
I_ctrl = 4 * 10^-3; % 4 mA

Q_sw1 = 60 * 10^-9; % 60 nF
Q_sw2 = 60 * 10^-9;
Q_sw3 = 60 * 10^-9;
Q_sw4 = 60 * 10^-9;

% Supcap
V_OC = Qacc / Cacc;

D = V_out / V_in;
delta_I = V_out * (1 - D)/(Lf * fs);

% R = simplify(delta_I);
R_temp = R_L + D*R_sw1 + (1-D)*R_sw2 + R_sw4;
P_conv = I_out^2 * (R_temp) + (1/12) * (delta_I)^2 * (R_temp + R_C) + V_in*fs*(Q_sw1 + Q_sw2) + V_in*I_ctrl;

F = [(V_in*I_in - P_conv - V_out*I_out) (V_OC - V_in - I_in * Racc)];
% simplify(f);





