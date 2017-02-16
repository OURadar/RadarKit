filename = boonlib('choosefile', '.', '*.rkr', [1 2]);
disp(filename)

dat = iqread(filename);

%%
disp(dat.pulses(40))

% Original data in I/Q, gate, channel, pulse count
pulses = cat(4, dat(:).pulses.iq);

% Marry I and Q into a complex number
pulses = complex(pulses(1, :, :, :), pulses(2, :, :, :));

% Reorder the indices to gate, pulse count, channel
pulses = permute(pulses, [2 4 3 1]);

pulses = single(pulses);

%%
N = 6;
ns = 400;
t = (1:ns) / 100e6;

figure(1)
clf
imagesc(20 * log10(abs(pulses(1:ns, 1:500, 2))))
set(gca, 'YDir', 'Normal')
colorbar
boonlib('bsizewin', gcf, [800 400])
xlim([0 100])

%%
figure(2)
for ii = 1:4 * N
    subplot(N, 4, ii)
    cplot(1e6 * t, pulses(1:ns, ii + 42, 2))
    if ii > 3 * N, xlabel('Time (us)'); end
end
boonlib('bsizewin', gcf, [1800 1300])
