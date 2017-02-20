filename = boonlib('choosefile', '~/Downloads', '*.rkr', [1 1]);
disp(filename)

dat = iqread(filename);

%%
disp(dat.pulses(1))

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
t = (1:ns) / 100;

ng = min(size(pulses, 2), 500);

figure(1)
clf
imagesc(20 * log10(abs(pulses(1:ns, 1:ng, 2))))
set(gca, 'YDir', 'Normal')
colorbar
boonlib('bsizewin', gcf, [800 400])
xlim([0 100])

%%
figure(2)
clf
M = 8;
ha = zeros(M * N, 1);
hd = zeros(M * N, 3);
ht = zeros(M * N, 1);
for ii = 1 : M * N
    k = ii;
    ix = rem(ii - 1, M);
    iy = N - 1 - floor((ii - 1) / M);
    ha(ii) = axes('Unit', 'Normalized', 'Position', [0.04 + ix / M * 0.94, 0.05 + iy / N * 0.94, 0.88 / M, 0.88 / N]);
    hd(ii, :) = plot(t, real(pulses(1:ns, k, 2)), ...
        t, imag(pulses(1:ns, k, 2)), ...
        t, abs(pulses(1:ns, k, 2)));
    grid on
    if iy == 0, xlabel('Time (us)'); else, set(gca, 'XTickLabel', []); end
    if ix == 0, ylabel('Amplitude'); else, set(gca, 'YTickLabel', []); end
    ht(ii) = text(0.9 * t(end), -5000, sprintf('%d %d / %d', dat.pulses(k).n, dat.pulses(k).i, rem(dat.pulses(k).i, 8)));
    ylim(5900 * [-1 1])
end
boonlib('bsizewin', gcf, [1920 1080])
set(ht, 'HorizontalAlignment', 'Right');

return

jj = 1;
while (jj < size(pulses, 2))
    for ii = 1 : M * N
        k = ii + 48 * (jj - 1);
        if k > size(pulses, 2), break; end;
        set(hd(ii, 1), 'YData', real(pulses(1:ns, k, 2)));
        set(hd(ii, 2), 'YData', imag(pulses(1:ns, k, 2)));
        set(hd(ii, 3), 'YData', abs(pulses(1:ns, k, 2)));
        set(ht(ii), 'String', sprintf('%d %d / %d', dat.pulses(k).n, dat.pulses(k).i, rem(dat.pulses(k).i, 8)));
    end
    drawnow
    bfig png
    jj = jj + 1;
end
