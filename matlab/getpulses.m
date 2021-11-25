function pulses = getpulses(dat)
    % Original data in I/Q, gate, channel, pulse count
    pulses = cat(4, dat.pulses(:).iq);

    % Marry I and Q into a complex number
    pulses = complex(pulses(1, :, :, :), pulses(2, :, :, :));

    % Reorder the indices to gate, pulse count, channel
    pulses = permute(pulses, [2 4 3 1]);

    pulses = single(pulses);
    