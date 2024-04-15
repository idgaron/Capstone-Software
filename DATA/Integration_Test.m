% Script to plot and calcuate the resulting data

data = readtable("POSTER_DATA.xlsx");
time_s = (data.time).*10^-6;
num = (data.sample_s);

graph = tiledlayout(4,2);

nexttile([1 2]);
plot(num, data.MFC_C);
xlim("tight")
ylim([0 3])
title('Control Patch Data');

nexttile([1 2]);
plot(num, data.MFC_E, 'r');
xlim("tight"); ylim("tight");
ylim([0 3])
title('Experimental Patch Data');

nexttile;
plot(num, data.MFC_C);
xlim("tight"); ylim("tight");
xlim([0.75 1])
ylim([0 3])
title('Response Before State Change');
ylabel('CONTROL', 'FontWeight', 'bold')

nexttile;
plot(num, data.MFC_C);
xlim("tight"); ylim("tight");
xlim([14.3 14.55])
ylim([0 3])
title('Response After State Change');

nexttile;
plot(num, data.MFC_E, 'r');
xlim("tight"); ylim("tight");
xlim([0.75 1])
ylim([0 3])
ylabel('EXPERIMENTAL', 'FontWeight', 'bold')

nexttile;
plot(num, data.MFC_E, 'r');
xlim("tight"); ylim("tight");
xlim([14.3 14.55])
ylim([0 3])