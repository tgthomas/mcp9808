% mcp9808 temperature sensor

% example data
% $ mcp9800 > temperature.log

fname = 'temperature.log';
fileID = fopen(fname,'r');
temp = fscanf(fileID,'%f');

n = length(temp);
t = (0:n-1)*(1./3600);

% smoothed trace, define dT noise as
% dT = temp - temp_ave
temp_ave = smooth(temp,21);

% time trace
figure(1)
hold off
plot(t,temp,'.-')
hold on
plot(t,temp_ave,'-r');
ylim([25.5, 26.5])
ylabel('Temperature [decG]');
xlabel('time [hr]');

% histogram dT
figure(2)
hold off
histogram(temp - temp_ave, 50);
hold on
ylabel('bin count');
xlabel('delta temperature [degC]');
