classdef iqread
    properties (Constant)
        constants = struct(...
            'RKNameLength', 256, ...
            'RKFileHeaderSize', 4096, ...
            'RKMaxMatchedFilterCount', 4, ...
            'RKMaximumStringLength', 4096, ...
            'RKMaximumPathLength', 1024, ...
            'RKRadarDesc', 1604, ...
            'RKMaxFilterCount', 8);
    end
    properties
        filename = '';
        header = struct('preface', [], 'buildNo', 0, 'desc', [], 'config', []);
        pulses = []
    end
    methods
        % Constructor
        function self = iqread(filename)
            fprintf('Filename: %s\n', filename);
            self.filename = filename;
            fid = fopen(self.filename);
            if (fid < 0)
                error('Unable to open file.');
            end
            
%             rk_config_size = 4;
%             rk_config_size = rk_config_size + 4 * 4 * self.RKMaxMatchedFilterCount;
%             rk_config_size = rk_config_size + self.RKMaximumStringLength;
%             rk_config_size = rk_config_size + 4 * 2 * 4;
%             rk_config_size = rk_config_size + 4;     % censorSNR
%             rk_config_size = rk_config_size + 2 * 4; % sweepElevation, sweepAzimuth
%             rk_config_size = rk_config_size + 4;     % RKMarker

            % Header
            self.header.preface = fread(fid, [1 self.constants.RKNameLength], 'char=>char');
            self.header.buildNo = fread(fid, 1, 'uint32');
            
            % Header->desc
            h = memmapfile(self.filename, ...
                'Offset', self.constants.RKNameLength + 4, ...    % RKNameLength * (char) + (uint32_t)
                'Repeat', 1, ...
                'Format', { ...
                    'uint32', [1 1], 'initFlags'; ...
                    'uint32', [1 1], 'pulseCapacity'; ...
                    'uint32', [1 1], 'pulseToRayRatio'; ...
                    'uint32', [1 1], 'healthNodeCount'; ...
                    'uint32', [1 1], 'configBufferDepth'; ...
                    'uint32', [1 1], 'positionBufferDepth'; ...
                    'uint32', [1 1], 'pulseBufferDepth'; ...
                    'uint32', [1 1], 'rayBufferDepth'; ...
                    'uint32', [1 1], 'controlCount'; ...
                    'double', [1 1], 'latitude'; ...
                    'double', [1 1], 'longitude'; ...
                    'single', [1 1], 'heading'; ...
                    'single', [1 1], 'radarHeight'; ...
                    'single', [1 1], 'wavelength'; ...
                    'uint8',  [1 self.constants.RKNameLength], 'name'; ...
                    'uint8',  [1 self.constants.RKNameLength], 'filePrefix'; ...
                    'uint8',  [1 self.constants.RKMaximumPathLength], 'dataPath'});
            self.header.desc = h.data;
            
            % Header->config
            c = memmapfile(self.filename, ...
                'Offset', self.constants.RKNameLength + 4 + self.constants.RKRadarDesc, ... % RKNameLength * (char) + (uint32_t) + (RKRadarDesc)
                'Repeat', 1, ...
                'Format', { ...
                    'uint64', [1 1], 'i'; ...
                    'uint32', [1 self.constants.RKMaxFilterCount], 'pw'; ...
                    'uint32', [1 self.constants.RKMaxFilterCount], 'prf'; ...
                    'uint32', [1 self.constants.RKMaxFilterCount], 'gateCount'; ...
                    'uint32', [1 self.constants.RKMaxFilterCount], 'waveformId'; ...
                    'single', [1 2], 'noise'; ...
                    'single', [self.constants.RKMaxFilterCount 2], 'ZCal'; ...
                    'single', [self.constants.RKMaxFilterCount 1], 'DCal'; ...
                    'single', [self.constants.RKMaxFilterCount 1], 'PCal'; ...
                    'single', [1 1], 'SNRThreshold'; ...
                    'single', [1 1], 'sweepElevation'; ...
                    'single', [1 1], 'sweepAzimuth'; ...
                    'uint32', [1 1], 'startMarker'; ...
                    'uint8',  [1 self.constants.RKNameLength], 'waveform'; ...
                    'uint8',  [1 self.constants.RKNameLength], 'vcpDefinition'});
            self.header.config = c.data;

            % Partially read the very first pulse
            fseek(fid, self.constants.RKFileHeaderSize, 'bof');
            pulse_header.i = fread(fid, 1, 'uint64');
            pulse_header.n = fread(fid, 1, 'uint64');
            pulse_header.t = fread(fid, 1, 'uint64');
            pulse_header.s = fread(fid, 1, 'uint32');
            pulse_header.capacity = fread(fid, 1, 'uint32');
            pulse_header.gateCount = fread(fid, 1, 'uint32');
            fclose(fid);

            % Some dimensions
            fprintf('gateCount = %d   capacity = %d\n', pulse_header.gateCount, pulse_header.capacity);
            fprintf('Reading pulses ...\n');

            % Pulses
            m = memmapfile(self.filename, ...
                'Offset', self.constants.RKFileHeaderSize, ...
                'Format', { ...
                    'uint64', [1 1], 'i'; ...
                    'uint64', [1 1], 'n'; ...
                    'uint64', [1 1], 't'; ...
                    'uint32', [1 1], 's'; ...
                    'uint32', [1 1], 'capacity'; ...
                    'uint32', [1 1], 'gateCount'; ...
                    'uint32', [1 1], 'marker'; ...
                    'uint32', [1 1], 'pulseWidthSampleCount'; ...
                    'uint64', [1 1], 'time_tv_sec'; ...
                    'uint64', [1 1], 'time_tv_usec'; ...
                    'double', [1 1], 'timeDouble'; ...
                    'uint8',  [1 4], 'rawAzimuth'; ...
                    'uint8',  [1 4], 'rawElevation'; ...
                    'uint16', [1 1], 'configIndex'; ...
                    'uint16', [1 1], 'configSubIndex'; ...
                    'uint16', [1 1], 'azimuthBinIndex'; ...
                    'single', [1 1], 'gateSizeMeters'; ...
                    'single', [1 1], 'elevationDegrees'; ...
                    'single', [1 1], 'azimuthDegrees'; ...
                    'single', [1 1], 'elevationVelocityDegreesPerSecond'; ...
                    'single', [1 1], 'azimuthVelocityDegreesPerSecond'; ...
                    'int16',  [2 pulse_header.gateCount 2], 'iq'});
            self.pulses = m.Data;
        end
    end
end
