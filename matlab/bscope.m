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
    
    % Pulse compression waveform
    fprintf('Deriving wave template ...\n')
    n = 335;             % sample count of the tx waveform
    k = 100;             % number of samples for averaging
    t = (0:n-1) / 5e6;   % time axis
    wav = mean(raw_pulses(1:335, 1:k, 1), 2);
    wav = wav ./ sqrt(sum(abs(wav) .^ 2));
end

if ~exist('debug', 'var')
    debug = true;
end

if ~exist('showPreview', 'var')
    showPreview = true;
end

%% Pulse Compression
fprintf('Performing pulse compression ...\n')
nfft = pow2(ceil(log2(size(raw_pulses, 1))));
xf = fft(raw_pulses, nfft, 1);
wf = fft(wav, nfft);
yf = conj(wf) .* xf;
y = ifft(yf, [], 1);

%% Ring Filter - 0.1 rad/sec Elliptical filter
fprintf('Applying ring filter ...\n')
b = [+0.852417, -4.259192, +8.515492, -8.515492, +4.259192, -0.852417];
a = [+1.000000, -4.624806, +8.563920, -7.934279, +3.676572, -0.681368];
y = filter(b, a, y, [], 2);

%% B-Scope with m-Pulse Averaging
fprintf('Calculating reflectivity ...\n')
m = 100;
[g, p, c] = size(y);
nblock = floor(p / m);
yy = reshape(y(1:gateCount, 1 : nblock * m, :), [gateCount, m, nblock, c]);
yy = squeeze(mean(abs(yy) .^ 2, 2));

% mean(mean(yy(floor(30/.06):floor(50/.06), 1:250, 1)))
noise = permute([127.9690, 29.5491].', [3, 2, 1]);
yy = max(yy - noise, 0);

%% Reflectivity
rr = (1:gateCount).' * .03;
z = 10 * log10(rr .^ 2 .* yy) - 26;

%% Plot
cmap = blib('zmap');
cmap(1, :) = 0;

ha = subplot(2, 1, 1);
imagesc(1:nblock, rr, z(:, :, 1))
xlabel('Ray Index')
ylabel('Range (km)')
set(gca, 'YDir', 'Normal')
colorbar
title('Quick B-Scope of Reflectivity (H)', 'FontSize', 14)

ha(2) = subplot(2, 1, 2);
imagesc(1:nblock, rr, z(:, :, 2))
xlabel('Ray Index')
ylabel('Range (km)')
set(gca, 'YDir', 'Normal')
colorbar
title('Quick B-Scope of Reflectivity (V)', 'FontSize', 14)

lp = linkprop(ha, {'XLim', 'YLim', 'CLim'});
caxis([0, 80])
colormap(cmap)
