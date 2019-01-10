pw = 50e-6;
fs = 160e6;
fc = 50.0e6;
m = 0.01;
n = pw * fs;

% Waveform samples
wav = [cos(2 * pi * m * (0:n-1)).' sin(2 * pi * m * (0:n-1)).'];
samples = int16(65000 * wav);

filename = 'xx.rkwav';


name = zeros(1, 256, 'uint8');
name(1:2) = 'xx';

%% 
% fwrite(&fileHeader, sizeof(RKWaveFileHeader), 1, fid);
% h = memmapfile(filename, ...
%     'Offset', 0, ...
%     'Repeat', 1, ...
%     'Format', { ...
%         'uint8', [1, 256], 'name'; ...
%         'uint8', [1, 1], 'groupCount'});
fid = fopen(filename, 'w');
if (fid < 0)
    error('Unable to open file')
end
b = fwrite(fid, name, 'char');
b = b + fwrite(fid, 1, 'uint8');
b = b + 4 * fwrite(fid, 1, 'uint32');
b = b + 8 * fwrite(fid, fc, 'double');
b = b + 8 * fwrite(fid, fs, 'double');
b = b + fwrite(fid, zeros(1, 512 - b, 'uint8'), 'uint8');
fprintf('Header with %d bytes\n', b);

% Only one group
for g = 1:1
    % fwrite(&groupHeader, sizeof(RKWaveFileGroup), 1, fid);
    waveformType = (2 ^ 0 + 2 ^ 5);
    waveformDepth = n;
    filterCount = 1;
    b = 4 * fwrite(fid, waveformType, 'uint32');
    b = b + 4 * fwrite(fid, waveformDepth, 'uint32');
    b = b + 4 * fwrite(fid, filterCount, 'uint32');
    b = b + 1 * fwrite(fid, zeros(1, 32 - b, 'uint8'), 'uint8');
    fprintf('Group header %d with %d bytes\n', g, b);
    % fwrite(waveform->filterAnchors[k], sizeof(RKFilterAnchor), groupHeader.filterCounts, fid);
    % b = fwrite(b, 
end
fclose(fid);
