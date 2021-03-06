if ~exist('filename', 'var'), filename = blib('choosefile', '~/Downloads', '*.rk*'); end
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
M = 6;   % Number of filters

% ng = min(size(pulses, 1), 30);
ng = min(size(pulses, 1), 1900);
ns = min(size(pulses, 2), 500);

if dat.header.buildNo >= 4
    if dat.header.dataType == 1
        % Compressed IQ
        dt = double(dat.header.desc.pulseToRayRatio) * double(dat.header.config.pulseGateSize) * 2 / 3.0e2;
    else
        % Raw IQ
        dt = double(dat.header.config.pulseGateSize) * 2 / 3.0e2;
    end  
else
    dt = 1 / 50;
end
t_fast = (1:ng) * dt;
t_slow = (1:ns) / 100;

figure(1)
clf
imagesc(20 * log10(abs(pulses(1:ng, 1:ns, 2))))
set(gca, 'YDir', 'Normal')
colorbar
blib('bsizewin', gcf, [800 400])
xlim([0 100])

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
    FIG.ax(ii) = axes('Unit', 'Normalized', 'Position', [0.02 + ix / M * 0.97, 0.05 + iy / N * 0.94, 0.92 / M, 0.88 / N]);
    FIG.pl(ii, :) = plot(...
        t_fast, real(pulses(1:ng, k, 2)), ...
        t_fast, imag(pulses(1:ng, k, 2)), ...
        t_fast, abs(pulses(1:ng, k, 2)));
    grid on
    if iy == 0, xlabel('Time (us)'); else, set(gca, 'XTickLabel', []); end
    if ix == 0, ylabel('Amplitude'); else, set(gca, 'YTickLabel', []); end
    iw = rem(int32(dat.pulses(k).i), M);
    iw2 = idivide(iw, 2);
    str = sprintf('[n:%d i:%d] / [%d / %d]', dat.pulses(k).n, dat.pulses(k).i, iw, iw2);
    FIG.ht(ii) = text(0.9 * t_fast(end), -12000, str);
    ylim(15000 * [-1 1])
end
% blib('bsizewin', gcf, [2500 1080])
blib('bsizewin', gcf, [1440 700])
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
            set(FIG.pl(ii, 1), 'YData', real(pulses(1:ng, k, 2)));
            set(FIG.pl(ii, 2), 'YData', imag(pulses(1:ng, k, 2)));
            set(FIG.pl(ii, 3), 'YData', abs(pulses(1:ng, k, 2)));
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
