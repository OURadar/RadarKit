filename = boonlib('choosefile', '~/Downloads', '*.rkr');

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
ng = min(size(pulses, 1), 75);
ns = min(size(pulses, 2), 500);

t_fast = (1:ng) / 50;
t_slow = (1:ns) / 100;

figure(1)
clf
imagesc(20 * log10(abs(pulses(1:ng, 1:ns, 2))))
set(gca, 'YDir', 'Normal')
colorbar
boonlib('bsizewin', gcf, [800 400])
xlim([0 100])

%%
figure(2)
clf
M = 22;
ha = zeros(M * N, 1);
hd = zeros(M * N, 3);
ht = zeros(M * N, 1);
for ii = 1 : M * N
    k = ii;
    ix = rem(ii - 1, M);
    iy = N - 1 - floor((ii - 1) / M);
    ha(ii) = axes('Unit', 'Normalized', 'Position', [0.02 + ix / M * 0.97, 0.05 + iy / N * 0.94, 0.92 / M, 0.88 / N]);
    hd(ii, :) = plot(...
        t_fast, real(pulses(1:ng, k, 2)), ...
        t_fast, imag(pulses(1:ng, k, 2)), ...
        t_fast, abs(pulses(1:ng, k, 2)));
    grid on
    if iy == 0, xlabel('Time (us)'); else, set(gca, 'XTickLabel', []); end
    if ix == 0, ylabel('Amplitude'); else, set(gca, 'YTickLabel', []); end
    ht(ii) = text(0.9 * t_fast(end), -10000, sprintf('%d / %d / %d', dat.pulses(k).n, dat.pulses(k).i, floor(rem(int32(dat.pulses(k).i), M) / 2) - 5));
    ylim(11500 * [-1 1])
end
boonlib('bsizewin', gcf, [3200 1080])
set(gcf, 'Menubar', 'None');
set(ht, 'HorizontalAlignment', 'Right');
lp = linkprop(ha, {'XLim'});

return

%%
%jj = 1;
%while (jj < size(pulses, 2))
    for ii = 1 : M * N
        k = ii + jj;
        if k > size(pulses, 2), break; end;
        set(hd(ii, 1), 'YData', real(pulses(1:ng, k, 2)));
        set(hd(ii, 2), 'YData', imag(pulses(1:ng, k, 2)));
        set(hd(ii, 3), 'YData', abs(pulses(1:ng, k, 2)));
        set(ht(ii), 'String', sprintf('%d (%d) / %d (%d)', ...
            dat.pulses(k).n, floor(rem(double(dat.pulses(k).n), M) / 2) - 5, ...
            dat.pulses(k).i, floor(rem(double(dat.pulses(k).i), M) / 2) - 5));
    end
    drawnow
    % bfig png
    jj = jj + M * N;
%end
