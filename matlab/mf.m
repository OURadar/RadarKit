if ~exist('dat', 'var')
    filename = blib('choosefile', '~/Downloads', '*.rkr');

    dat = iqread(filename);

    fprintf('Rearranging data ...\n');

    % 
    disp(dat.pulses(1))

    % Original data in I/Q, gate, channel, pulse count
    pulses = cat(4, dat.pulses(:).iq);

    % Marry I and Q into a complex number
    pulses = complex(pulses(1, :, :, :), pulses(2, :, :, :));

    % Reorder the indices to gate, pulse count, channel
    pulses = permute(pulses, [2 4 3 1]);

    pulses = single(pulses);
end

depth = 25;
fs = 20.0e6;
waveform = 'h2005';

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
    [0, 1, 2], ... hopeCount = 3
    [0, 1, 2, 3], ... hopeCount = 4
    [0, 1, 2, 3, 4], ... hopeCount = 5
    [0, 1, 2, 3, 4, 5], ... hopeCount = 6
    [0, 2, 4, 6, 1, 3, 5], ... hopeCount = 7
    [0, 2, 4, 6, 0, 2, 4, 6], ... hopeCount = 8
    [0, 2, 4, 6, 8, 1, 3, 5, 7], ... hopeCount = 9
    [0, 2, 4, 6, 8, 0, 2, 4, 6, 8], ... hopeCount = 10
    [0, 3, 6, 9, 1, 4, 7, 10, 2, 5, 8], ... hopeCount = 11
    [0, 3, 6, 9, 0, 3, 6, 9, 0, 3, 6, 9], ... hopeCount = 12
    [0, 3, 6, 9, 12, 2, 5, 8, 11, 1, 4, 7, 10], ... hopeCount = 13
    };
    
stride = strideCollection{hopCount};

if hopCount <= 2
    fprintf('Unable to continue.\n')
    return
else
    delta = bandwidth / (hopCount - 1);
end

%         f = delta * (double)n - 0.5 * bandwidth + fc;
%         omega = 2.0 * M_PI * f / fs;
%         psi = omega * (double)(waveform->depth / 2);

f = delta * stride - 0.5 * bandwidth;
omega = 2.0 * pi * f / fs;
omega_n = 0.5 * omega(:) * (0:depth - 1);
psi = 0.5 * omega(:) * ones(1, depth);
% w = exp(1i * (omega_n - psi_n)) / sqrt(depth);
w = cos(omega_n - psi) + 1i * sin(omega_n - psi);
gain = 10 * log10(sum(abs(w) .^ 2, 2)) .';
fprintf('%d   f = %7.3f MHz --> %7.3f rad/sample   noise gain = %.2f dB\n', [stride; 1.0e-6 * f; omega; gain]);

ww = kron(w, [1; 1]);

%%
clf
M = 10;
N = 2;
FIG.ax = zeros(M * N, 1);
FIG.pl = zeros(M * N, 3);
FIG.ht = zeros(M * N, 1);

for ii = 1 : M * N
    k = ii;
    ix = rem(ii - 1, M);
    iy = N - 1 - floor((ii - 1) / M);
    FIG.ax(ii) = axes('Unit', 'Normalized', 'Position', [0.02 + ix / M * 0.97, 0.05 + iy / N * 0.94, 0.92 / M, 0.88 / N]);
    
    if iy == 0
        cplot(ww(ix + 1, :))
    else
        cplot(1.0e-4 * pulses(1 : depth, ix + 2, 1))
    end
    set(FIG.ax(ii), 'XLim', [0 25])
    
    if (ix > 0)
        set(FIG.ax(ii), 'YTickLabel', [])
    end
    if (iy > 0)
        set(FIG.ax(ii), 'XTickLabel', [])
    end
end


%% Find the sequence anchor
ccf = zeros(1, 2 * hopCount);
ww_mag = sqrt(sum(abs(ww(:)) .^ 2));
for k = 1:14
    l = k + 2 * hopCount - 1;
    tx = pulses(1:depth, k:l, 1);
    tx_mag = sqrt(sum(abs(tx(:)) .^ 2));
    ccf(k) = sum(conj(tx(:)) .* ww(:)) / ww_mag / tx_mag;
end
