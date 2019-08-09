pw = 50e-6;
fs = 160e6;
fc = 50.0e6;
m = 0.01;
n = pw * fs;

% Waveform samples
wav = exp(1i * 2 * pi * m * (0:n-1));
wav = single(wav);
wav = [real(wav); imag(wav)];
samples = int16(65000 * wav);

filename = 'tone.rkwav';


name = zeros(1, 256, 'uint8');
name(1:2) = 'xx';

fid = fopen(filename, 'w');
if (fid < 0)
    error('Unable to open file')
end
depth = n;
b = fwrite(fid, name, 'char');
b = b + fwrite(fid, 1, 'uint8');
b = b + 4 * fwrite(fid, depth, 'uint32');
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
    name = 0;
    origin = 0;
    length = n;
    inputOrigin = 0;
    outputOrigin = 0;
    maxDataLength = 2 ^ 18;
    subCarrierFrequency = 0.0;
    sensitivityGain = pw;
    filterGain = 1.0;
    fullScale = 65000;
    b = 4 * fwrite(fid, 0, 'uint32');
    b = b + 4 * fwrite(fid, origin, 'uint32');
    b = b + 4 * fwrite(fid, length, 'uint32');
    b = b + 4 * fwrite(fid, inputOrigin, 'uint32');
    b = b + 4 * fwrite(fid, outputOrigin, 'uint32');
    b = b + 4 * fwrite(fid, maxDataLength, 'uint32');
    b = b + 4 * fwrite(fid, subCarrierFrequency, 'float');
    b = b + 4 * fwrite(fid, sensitivityGain, 'float');
    b = b + 4 * fwrite(fid, filterGain, 'float');
    b = b + 4 * fwrite(fid, fullScale, 'float');
    b = b + 1 * fwrite(fid, zeros(1, 64 - b, 'uint8'), 'uint8');
    fprintf('Filter anchor %d with %d bytes\n', g, b);
    % fwrite(waveform->samples[k], sizeof(RKComplex), groupHeader.depth, fid);
    % fwrite(waveform->iSamples[k], sizeof(RKInt16C), groupHeader.depth, fid);
    b = fwrite(fid, wav, 'float');
    if (b ~= 2 * n)
        fprintf('Unexpected return from float write.  b = %d\n', b);
    end
    b = fwrite(fid, samples, 'int16');
    if (b ~= 2 * n)
        fprintf('Unexpected return from float write.  b = %d\n', b);
    end
end
fclose(fid);
