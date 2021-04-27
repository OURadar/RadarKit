if ~exist('filename', 'var'), filename = blib('choosefile', '~/Downloads/raxpol', '*.rk*'); end
if ~exist('showAnimation', 'var'), showAnimation = false; end
if ~exist('generatePNG', 'var'), generatePNG = false; end

dat = iqread(filename);

fprintf('Rearranging data ...\n');

%
% disp(dat.pulses(1))

% Original data in I/Q, gate, channel, pulse count
pulses = cat(4, dat.pulses(:).iq);

% Marry I and Q into a complex number
pulses = complex(pulses(1, :, :, :), pulses(2, :, :, :));

% Reorder the indices to gate, pulse count, channel
pulses = permute(pulses, [2 4 3 1]);

pulses = single(pulses);

%%
N = 4;   % Number of rows
M = 1;   % Number of filters

% ng = min(size(pulses, 1), 30);
ng = min(size(pulses, 1), 1600);
ns = min(size(pulses, 2), 8000);

% Compute dt sampling interval in us
if dat.header.buildNo >= 4
    if dat.header.dataType == 1
        % Raw IQ straight from the transceiver
        dr = double(dat.header.config.pulseGateSize);
        dt = dr * 2 / 3.0e2;
    elseif dat.header.dataType == 2
        % Compressed IQ
        dr = double(dat.header.desc.pulseToRayRatio) * double(dat.header.config.pulseGateSize);
        dt = dr * 2 / 3.0e2;
    else
        fprintf('Inconsistency detected. This should not happen\n');
        % Assume 50-MHz sampling
        dr = 30;
        dt = 1 / 50;
    end  
else
    % Assume 50-MHz sampling
    dr = 30;
    dt = 1 / 50;
end

ds = floor(ns / 800);

r_fast = (1:ng) * dr;
t_fast = (1:ng) * dt;
t_slow = (1:ds:ns) * dat.header.config.prt(1);

% Initialize a figure
figure(1)
clf
imagesc(t_slow, 1.0e-3 * r_fast, 20 * log10(abs(pulses(1:ng, 1:ds:ns, 2))))
xlabel('Time (ms)')
ylabel('Range (km)')
set(gca, 'YDir', 'Normal')
colorbar
blib('bsizewin', gcf, [800 600])
caxis([10 40])

%%
figure(2)
clf
FIG.ax = zeros(M * N, 1);
FIG.pl = zeros(M * N, 3);
FIG.ht = zeros(M * N, 1);

% Compute the next pulse index that have rem(i, M) = 0
offset = 0;
if rem(dat.pulses(1).i, M) > 0
    offset = M - rem(dat.pulses(1).i, M);
end

for ii = 1 : M * N
    k = ii + offset;
    ix = rem(ii - 1, M);
    iy = N - 1 - floor((ii - 1) / M);
    FIG.ax(ii) = axes('Unit', 'Normalized', 'Position', [0.04 + ix / M * 0.97, 0.05 + iy / N * 0.94, 0.92 / M, 0.88 / N]);
    FIG.pl(ii, :) = plot(...
        t_fast, 1e-4 * real(pulses(1:ng, k, 2)), ...
        t_fast, 1e-4 * imag(pulses(1:ng, k, 2)), ...
        t_fast, 1e-4 * abs(pulses(1:ng, k, 2)), 'LineWidth', 1.5);
    grid on
    if iy == 0, xlabel('Time (us)'); else, set(gca, 'XTickLabel', []); end
    if ix == 0, ylabel('Amplitude'); else, set(gca, 'YTickLabel', []); end
    iw = rem(int32(dat.pulses(k).i), M);
    iw2 = idivide(iw, 2);
    str = sprintf('[n:%d i:%d] / [%d / %d]', dat.pulses(k).n, dat.pulses(k).i, iw, iw2);
    FIG.ht(ii) = text(0.95 * t_fast(end), -1.2e3, str);
    ylim(1.19 * [-1 1])
end
% blib('bsizewin', gcf, [2500 1080])
blib('bsizewin', gcf, [1600 890])
set(gcf, 'Menubar', 'None');
set(FIG.ht, 'HorizontalAlignment', 'Right');
lp = linkprop(FIG.ax, {'XLim', 'YLim'});
xlim([0 t_fast(ng)])

if (showAnimation)
    %%
    jj = 1; 
    while (jj < size(pulses, 2))
        for ii = 1 : M * N
            k = ii + jj;
            if k > size(pulses, 2), break; end
            set(FIG.pl(ii, 1), 'YData', 1e-4 * real(pulses(1:ng, k, 2)));
            set(FIG.pl(ii, 2), 'YData', 1e-4 * imag(pulses(1:ng, k, 2)));
            set(FIG.pl(ii, 3), 'YData', 1e-4 * abs(pulses(1:ng, k, 2)));
            set(FIG.ht(ii), 'String', sprintf('%d (%d) / %d (%d)', ...
                dat.pulses(k).n, floor(rem(double(dat.pulses(k).n), M) / 2) - 5, ...
                dat.pulses(k).i, floor(rem(double(dat.pulses(k).i), M) / 2) - 5));
        end
        drawnow
        if (generatePNG)
            bfig png
        end
        jj = jj + M * N;
    end
end
