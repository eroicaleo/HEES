function F = dcdc_model_to_battery_boost(x)

V_out = x(1);
I_out = x(2);
% syms I_out;
% syms V_out;
% syms V_in I_in;
global V_in I_in V_SOC;

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

% Battery
b11 = -0.67; b12 = -16.21; b13 = -0.03; b14 = 1.28; b15 = -0.40; b16 = 7.55;
b21 = 0.10; b22 = -4.32; b23 = 0.34;
b31 = 0.15; b32 = -19.6; b33 = 0.19;
b51 = 2.07; b52 = -190.41; b53 = 0.2;
V_OC = b11*exp(b12*V_SOC) + b13*V_SOC^3 + b14*V_SOC^2 + b15*V_SOC + b16;
R_s = b21*exp(b22*V_SOC) + b23;
R_ts = b31*exp(b32*V_SOC) + b33;
R_tl = b51*exp(b52*V_SOC) + b53;

D = 1 - V_in / V_out;
delta_I = V_in * D/(Lf * fs);

% R = simplify(delta_I);
R_temp = R_L + D*R_sw3 + (1-D)*R_sw4 + R_sw1;
P_conv = (I_out/(1-D))^2 * (R_temp + D*(1-D)*R_C) + (1/12) * (delta_I)^2 * (R_temp + (1-D)*R_C) + V_out*fs*(Q_sw3 + Q_sw4) + V_in*I_ctrl;

F = [(V_in*I_in - P_conv - V_out*I_out) (V_out - V_OC - I_out * (R_s + R_ts + R_tl))];
% simplify(f);





