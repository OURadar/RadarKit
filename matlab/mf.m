% Run the script quiet.m to suppress all the debuggning and plot routines
eval('quiet')

if ~exist('dat', 'var')
    filename = blib('choosefile', '~/Downloads', '*.rkr');

    dat = iqread(filename);

    fprintf('Rearranging data ...\n');

    % 
    disp(dat.pulses(1))

    % Original data in I/Q, gate, channel, pulse count
    raw_pulses = cat(4, dat.pulses(:).iq);

    % Marry I and Q into a complex number
    raw_pulses = complex(raw_pulses(1, :, :, :), raw_pulses(2, :, :, :));

    % Reorder the indices to gate, pulse count, channel
    raw_pulses = permute(raw_pulses, [2 4 3 1]);

    raw_pulses = single(raw_pulses);

    % Basic dimentions
    [gateCount, pulseCount, channelCount] = size(raw_pulses);
end

if ~exist('debug', 'var')
    debug = true;
end

if ~exist('showPreview', 'var')
    showPreview = true;
end

% Seems like the waveform name was not recorded, need to override this manually
fs = 20.0e6;
waveform = 'h2007.5';

% Waveform from name
if waveform(1) == 'h'
    bandwidth = 1.0e6 * str2double(waveform(2:3));
    hopCount = str2double(waveform(4:5));
    if length(waveform) > 5
        pulsewidth = 1.0e-6 * str2double(waveform(6:end));
    else
        pulsewidth = 0.5e-6;
    end
end
pulseWidthSampleCount = pulsewidth * fs;

% These are results from RadarKit:
% ==============
% RKTestMakeHops
% ==============
% 3 Hops:
% stride = 1   m = 1 1 0   score = 9.25
% stride = 2   m = 1 1 0   score = 9.25
%     Best stride = 1  ==>  0 1 2
% 
% 4 Hops:
% stride = 1   m = 1 2 1   score = 27.00
% stride = 2   m = 2 0 2   score = 20.00
% stride = 3   m = 1 2 1   score = 27.00
%     Best stride = 1  ==>  0 1 2 3
% 
% 5 Hops:
% stride = 1   m = 1 2 2   score = 40.50
% stride = 2   m = 2 1 1   score = 38.25
% stride = 3   m = 2 1 1   score = 38.25
% stride = 4   m = 1 2 2   score = 40.50
%     Best stride = 1  ==>  0 1 2 3 4
% 
% 6 Hops:
% stride = 1   m = 1 2 3   score = 55.00
% stride = 2   m = 2 2 0   score = 43.00
% stride = 3   m = 3 0 3   score = 48.00
% stride = 4   m = 2 2 0   score = 43.00
% stride = 5   m = 1 2 3   score = 55.00
%     Best stride = 1  ==>  0 1 2 3 4 5
% 
% 7 Hops:
% stride = 1   m = 1 2 3   score = 67.00
% stride = 2   m = 2 3 1   score = 77.25
% stride = 3   m = 3 1 2   score = 76.25
% stride = 4   m = 3 1 2   score = 76.25
% stride = 5   m = 2 3 1   score = 77.25
% stride = 6   m = 1 2 3   score = 67.00
%     Best stride = 2  ==>  0 2 4 6 1 3 5
% 
% 8 Hops:
% stride = 1   m = 1 2 3   score = 79.00
% stride = 2   m = 2 4 2   score = 108.00
% stride = 3   m = 3 2 1   score = 95.00
% stride = 4   m = 4 0 4   score = 88.00
% stride = 5   m = 3 2 1   score = 95.00
% stride = 6   m = 2 4 2   score = 108.00
% stride = 7   m = 1 2 3   score = 79.00
%     Best stride = 2  ==>  0 2 4 6 0 2 4 6
% 
% 9 Hops:
% stride = 1   m = 1 2 3   score = 91.00
% stride = 2   m = 2 4 3   score = 134.50
% stride = 3   m = 3 3 0   score = 101.25
% stride = 4   m = 4 1 3   score = 126.25
% stride = 5   m = 4 1 3   score = 126.25
% stride = 6   m = 3 3 0   score = 101.25
% stride = 7   m = 2 4 3   score = 134.50
% stride = 8   m = 1 2 3   score = 91.00
%     Best stride = 2  ==>  0 2 4 6 8 1 3 5 7
% 
% 10 Hops:
% stride = 1   m = 1 2 3   score = 103.00
% stride = 2   m = 2 4 4   score = 162.00
% stride = 3   m = 3 4 1   score = 152.00
% stride = 4   m = 4 2 2   score = 153.00
% stride = 5   m = 5 0 5   score = 140.00
% stride = 6   m = 4 2 2   score = 153.00
% stride = 7   m = 3 4 1   score = 152.00
% stride = 8   m = 2 4 4   score = 162.00
% stride = 9   m = 1 2 3   score = 103.00
%     Best stride = 2  ==>  0 2 4 6 8 0 2 4 6 8
% 
% 11 Hops:
% stride =  1   m = 1 2 3   score = 115.00
% stride =  2   m = 2 4 5   score = 190.50
% stride =  3   m = 3 5 2   score = 196.25
% stride =  4   m = 4 3 1   score = 176.25
% stride =  5   m = 5 1 4   score = 188.25
% stride =  6   m = 5 1 4   score = 188.25
% stride =  7   m = 4 3 1   score = 176.25
% stride =  8   m = 3 5 2   score = 196.25
% stride =  9   m = 2 4 5   score = 190.50
% stride = 10   m = 1 2 3   score = 115.00
%     Best stride = 3  ==>  0 3 6 9 1 4 7 10 2 5 8
% 
% 12 Hops:
% stride =  1   m = 1 2 3   score = 127.00
% stride =  2   m = 2 4 6   score = 220.00
% stride =  3   m = 3 6 3   score = 243.00
% stride =  4   m = 4 4 0   score = 184.00
% stride =  5   m = 5 2 3   score = 223.00
% stride =  6   m = 6 0 6   score = 204.00
% stride =  7   m = 5 2 3   score = 223.00
% stride =  8   m = 4 4 0   score = 184.00
% stride =  9   m = 3 6 3   score = 243.00
% stride = 10   m = 2 4 6   score = 220.00
% stride = 11   m = 1 2 3   score = 127.00
%     Best stride = 3  ==>  0 3 6 9 0 3 6 9 0 3 6 9
% 
% 13 Hops:
% stride =  1   m = 1 2 3   score = 139.00
% stride =  2   m = 2 4 6   score = 244.00
% stride =  3   m = 3 6 4   score = 282.50
% stride =  4   m = 4 5 1   score = 251.25
% stride =  5   m = 5 3 2   score = 254.25
% stride =  6   m = 6 1 5   score = 262.25
% stride =  7   m = 6 1 5   score = 262.25
% stride =  8   m = 5 3 2   score = 254.25
% stride =  9   m = 4 5 1   score = 251.25
% stride = 10   m = 3 6 4   score = 282.50
% stride = 11   m = 2 4 6   score = 244.00
% stride = 12   m = 1 2 3   score = 139.00
%     Best stride = 3  ==>  0 3 6 9 12 2 5 8 11 1 4 7 10

strideCollection = { ...
    [], ... % hopCount = 1
    [], ... % hopCount = 2
    [0, 1, 2], ... % hopeCount = 3
    [0, 1, 2, 3], ... % hopeCount = 4
    [0, 1, 2, 3, 4], ... % hopeCount = 5
    [0, 1, 2, 3, 4, 5], ... % hopeCount = 6
    [0, 2, 4, 6, 1, 3, 5], ... % hopeCount = 7
    [0, 2, 4, 6, 0, 2, 4, 6], ... % hopeCount = 8
    [0, 2, 4, 6, 8, 1, 3, 5, 7], ... % hopeCount = 9
    [0, 2, 4, 6, 8, 0, 2, 4, 6, 8], ... % hopeCount = 10
    [0, 3, 6, 9, 1, 4, 7, 10, 2, 5, 8], ... % hopeCount = 11
    [0, 3, 6, 9, 0, 3, 6, 9, 0, 3, 6, 9], ... % hopeCount = 12
    [0, 3, 6, 9, 12, 2, 5, 8, 11, 1, 4, 7, 10], ... % hopeCount = 13
    };
n = strideCollection{hopCount};

if hopCount <= 2
    fprintf('Unable to continue.\n')
    return
else
    delta = bandwidth / (hopCount - 1);
end

% Generate the match filters
f = delta * n - 0.5 * bandwidth;
omega = 2.0 * pi * f / fs;
omega_n = omega(:) * (0:pulseWidthSampleCount - 1);
psi = repmat(0.5 * omega(:) * pulseWidthSampleCount, [1, pulseWidthSampleCount]);
w = exp(1i * (omega_n - psi)) / sqrt(pulseWidthSampleCount);
gain = 10 * log10(sum(abs(w) .^ 2, 2)) .';
fprintf('%d   f = %7.3f MHz --> %7.3f rad/sample   noise gain = %.2f dB\n', [n; 1.0e-6 * f; omega; gain]);

% Make ww same convention as the raw_pulses
ww = kron(w, [1; 1]).';

%% Find the sequence anchor
ww_t = ww(1:pulseWidthSampleCount - 1, :);
ccf = zeros(1, 2 * hopCount);
ww_mag = sqrt(sum(abs(ww_t(:)) .^ 2));
for k = 1:2 * hopCount
    l = k + 2 * hopCount - 1;
    tx = raw_pulses(1:pulseWidthSampleCount - 1, k:l, 1);
    tx_mag = sqrt(sum(abs(tx(:)) .^ 2));
    ccf(k) = sum((tx(:)) .* conj(ww_t(:))) / ww_mag / tx_mag;
end
[~, lag] = max(abs(ccf));
fprintf('Best anchor @ lag = %d\n', lag);

% Show some I/Q data
if (showPreview)
    figure(1)
    clf
    M = 2 * hopCount;
    N = 2;
    FIG.ax = zeros(M * N, 1);
    FIG.pl = zeros(M * N, 3);
    FIG.ht = zeros(M * N, 1);
    t = 0:(pulseWidthSampleCount - 1);

    cplot = @(x, y) plot(x, real(y), x, imag(y));

    a = 1.0 / 32.0e3 / sqrt(pulseWidthSampleCount);

    for ii = 1 : M * N
        k = ii;
        ix = rem(ii - 1, M);
        iy = N - 1 - floor((ii - 1) / M);
        FIG.ax(ii) = axes('Unit', 'Normalized', 'Position', [0.02 + ix / M * 0.97, 0.05 + iy / N * 0.94, 0.92 / M, 0.88 / N]);
        
        if iy == 0
            cplot(t, ww(1 : pulseWidthSampleCount, 1 + ix))
        else
            cplot(t, a * raw_pulses(1 : pulseWidthSampleCount, lag + ix, 1))
        end
        grid on
        
        if (ix > 0)
            set(FIG.ax(ii), 'YTickLabel', [])
        end
        if (iy > 0)
            set(FIG.ax(ii), 'XTickLabel', [])
        end
    end
    FIG.lp = linkprop(FIG.ax, {'XLim', 'YLim'});
    set(FIG.ax, 'XLim', [0 1.1 * pulseWidthSampleCount])
end

% Fourier representation of the data and matched filter
nfft = pow2(ceil(log2(size(raw_pulses, 1))));
wf = fft(ww, nfft, 1);
xf = fft(raw_pulses, nfft, 1);

% Repeat wf for V channel
wf = repmat(wf, [1, 1, channelCount]);

% Debugging mode
if (debug)
    pulseCount = min(pulseCount, 20);
    figure(2)
    t = 0:(pulseWidthSampleCount - 1);
end

% Pulses through matched filter should have the same dimensions as raw_pulses
pulses = zeros(size(raw_pulses));

% Match filtering
for k = 1:pulseCount
    % The correct index of the matched filter to use
    j = rem(k + 2 * hopCount - lag, 2 * hopCount) + 1;
    % Match filter using freq. domain
    yf = conj(wf(:, j, :)) .* xf(:, k, :);
    yn = ifft(yf, nfft, 1);
    pulses(:, k, :) = yn(1:gateCount, :, :);
    % Show the I/O during debugging mode
    if (debug)
        fprintf('k = %d   j = %d\n', k, j);
        subplot(3, 1, 1)
        cplot(t, ww(1:pulseWidthSampleCount, j));
        title('Filter')
        subplot(3, 1, 2)
        cplot(t, raw_pulses(1:pulseWidthSampleCount, k, 1));
        title('Data')
        subplot(3, 1, 3)
        cplot(t, pulses(1:pulseWidthSampleCount, k, 1));
        title('Output')
        pause
    end
end
