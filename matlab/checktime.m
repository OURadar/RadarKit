if ~exist('dat', 'var')
    filename = boonlib('choosefile', '~/Downloads', '*.rkr');

    dat = iqread(filename, 200);

    fprintf('Rearranging data ...\n');

    %%
    disp(dat.pulses(1))

    % Original data in I/Q, gate, channel, pulse count
    pulses = cat(4, dat.pulses(:).iq);

    % Marry I and Q into a complex number
    pulses = complex(pulses(1, :, :, :), pulses(2, :, :, :));

    % Reorder the indices to gate, pulse count, channel
    pulses = permute(pulses, [2 4 3 1]);

    pulses = single(pulses);
end

tv_sec = cat(1, dat.pulses(:).time_tv_sec);
tv_usec = cat(1, dat.pulses(:).time_tv_usec);
tv = double(tv_sec) + 1.0e-6 * double(tv_usec);
td = cat(1, dat.pulses(:).timeDouble);


%% 
n = numel(td) - 1;
figure(1003)
clf
FIG.lines = plot( 1:n, 1e3 * diff(tv), '.', 1:n, 1e3 * diff(td), '.');
grid on
legend('Raw - tv', 'RadarKit - td')
xlabel('Sample Index')
ylabel('Time (usec)')
title('Time Difference Between Samples')
FIG.prop = linkprop(FIG.lines, {'XLim', 'YLim'});
set(FIG.lines, 'MarkerSize', 18)
xlim([50 100])
pan on

