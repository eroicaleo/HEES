function power = dcdc_model_compute_power(V_in, I_in, V_out, I_out)

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

if (V_in >= V_out)
    D = V_out / V_in;
    delta_I = V_out * (1 - D)/(Lf * fs);

    R_temp = R_L + D*R_sw1 + (1-D)*R_sw2 + R_sw4;
    P_conv = I_out^2 * (R_temp) + (1/12) * (delta_I)^2 * (R_temp + R_C) + V_in*fs*(Q_sw1 + Q_sw2) + V_in*I_ctrl;
elseif (V_in < V_out)
    D = 1 - V_in / V_out;
    delta_I = V_in * D/(Lf * fs);

    R_temp = R_L + D*R_sw3 + (1-D)*R_sw4 + R_sw1;
    P_conv = (I_out/(1-D))^2 * (R_temp + D*(1-D)*R_C) + (1/12) * (delta_I)^2 * (R_temp + (1-D)*R_C) + V_out*fs*(Q_sw3 + Q_sw4) + V_in*I_ctrl;
end

power = P_conv;
