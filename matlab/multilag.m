N = 3;
va = pi;

Xh = [ 0+0j, 0+2j, 0+1j, 0+4j, 0+2j, 0+3j, 0+0j, 0+2j, 0+1j, 0+4j;...
 -1+2j, 1+0j, 0+1j, 3-2j, 1+0j, 2-1j, -1+2j, 1+0j, 0+1j, 3-2j;...
 4+0j, 0-2j, 2-1j, -4-4j, 0-2j, -2-3j, 4+0j, 0-2j, 2-1j, -4-4j;...
 -3+2j, 3+0j, 0+1j, 9-2j, 3+0j, 6-1j, -3+2j, 3+0j, 0+1j, 9-2j;...
 8+0j, 0-6j, 4-3j, -8-12j, 0-6j, -4-9j, 8+0j, 0-6j, 4-3j, -8-12j;...
 -5+2j, 5+0j, 0+1j, 15-2j, 5+0j, 10-1j, -5+2j, 5+0j, 0+1j, 15-2j];
  
Xv = [ 1-1j, 1+1j, 1+0j, 1+3j, 1+1j, 1+2j, 1-1j, 1+1j, 1+0j, 1+3j;...
 -2+4j, 0+2j, -1+3j, 2+0j, 0+2j, 1+1j, -2+4j, 0+2j, -1+3j, 2+0j;...
 7+1j, 3-1j, 5+0j, -1-3j, 3-1j, 1-2j, 7+1j, 3-1j, 5+0j, -1-3j;...
 -6+6j, 0+4j, -3+5j, 6+2j, 0+4j, 3+3j, -6+6j, 0+4j, -3+5j, 6+2j;...
 13+3j, 5-3j, 9+0j, -3-9j, 5-3j, 1-6j, 13+3j, 5-3j, 9+0j, -3-9j;...
 -10+8j, 0+6j, -5+7j, 10+4j, 0+6j, 5+5j, -10+8j, 0+6j, -5+7j, 10+4j];

% For lazy coding, in-place cross-correlation calculation
ixc = @(x, y, n) squeeze(mean(x(:, 1+n:end) .* conj(y(:, 1:end-n)), 2));
ixcn = @(x, y, m) squeeze(mean(x(:, 1:end+m) .* conj(y(:, 1-m:end)), 2));

ngate = size(Xh, 1);

% Pre-allocate, use similar format as the data
Rh = zeros(ngate, N + 1, 'single');
Rv = zeros(ngate, N + 1, 'single');
C = zeros(ngate, 2 * N + 1, 'single');
gC = zeros(ngate, 1, 'single');

for ilag = 0:N
    Rh(1:ngate, ilag + 1) = ixc(Xh, Xh, ilag);
    Rv(1:ngate, ilag + 1) = ixc(Xv, Xv, ilag); 
end

for ilag = -N:N
    if ilag < 0
        % Negative lags
        C(1:ngate, ilag + N + 1) = ixcn(Xh, Xv, ilag);
    else
        % Positive lags
        C(1:ngate, ilag + N + 1) = ixc(Xh, Xv, ilag);
    end
end

for g = 1:ngate
    aC = xcorr(Xh(g, :), Xv(g, :), N, 'unbiased') .';
    wn = 3 * N^2 + 3 * N - 1 - 5 * (-N : N).^2;
    wd = 3 / ((2 * N - 1) * (2 * N + 1) * (2 * N + 3));
    gC(g) = exp(wd * sum(wn(:) .* log(abs(aC))));
end


fprintf('Rh =\n')
disp(Rh.');

fprintf('|Rh| =\n')
disp(abs(Rh).');

fprintf('Rv =\n')
disp(Rv.');

fprintf('|Rv| =\n')
disp(abs(Rv).');

fprintf('C = \n')
disp(C.');

fprintf('gC = \n')
disp(gC.');

if N == 2
    % Lag-2 estimator
    Sh = (abs(Rh(:, 2)) .^ (4 / 3) ./ ...
          abs(Rh(:, 3)) .^ (1 / 3));
    Sv = (abs(Rv(:, 2)) .^ (4 / 3) ./ ...
          abs(Rv(:, 3)) .^ (1 / 3));
elseif N == 3
    % Lag-3 estimator
    Sh = (abs(Rh(:, 2)) .^ (6 / 7) .* abs(Rh(:, 3)) .^ (3 / 7) ./ ...
          abs(Rh(:, 4)) .^ (2 / 7));
    Sv = (abs(Rv(:, 2)) .^ (6 / 7) .* abs(Rv(:, 3)) .^ (3 / 7) ./ ...
          abs(Rv(:, 4)) .^ (2 / 7));
elseif N == 4
    Sh = (abs(Rh(:, 2)) .^ (54 / 86) .* abs(Rh(:, 3)) .^ (39 / 86) .* abs(Rh(:, 4)) .^ (14 / 86) ./ ...
          abs(Rh(:, 5)) .^ (21 / 86));
    Sv = (abs(Rv(:, 2)) .^ (54 / 86) .* abs(Rv(:, 3)) .^ (39 / 86) .* abs(Rv(:, 4)) .^ (14 / 86) ./ ...
          abs(Rv(:, 5)) .^ (21 / 86));
end

Vh = -va / pi * angle(Rh(:, 2));
Vv = -va / pi * angle(Rv(:, 2));

% Try to make widthFactor = 1.0
lambda = 1.0;
prt = 1 / 2 / sqrt(2) / pi;

if N == 2
    Wh = lambda/(sqrt(24) * pi * prt) * sqrt( log(abs(Rh(:, 2)) ./ abs(Rh(:, 3))) );
    Wv = lambda/(sqrt(24) * pi * prt) * sqrt( log(abs(Rv(:, 2)) ./ abs(Rv(:, 3))) );
elseif N == 3
    Wh = lambda/(28 * pi * prt) * sqrt( 11 * log(abs(Rh(:, 2))) + 2 * log(abs(Rh(:, 3))) - 13 * log(abs(Rh(:, 4))) );
    Wv = lambda/(28 * pi * prt) * sqrt( 11 * log(abs(Rv(:, 2))) + 2 * log(abs(Rv(:, 3))) - 13 * log(abs(Rv(:, 4))) );
elseif N == 4
    Wh = lambda/(4 * sqrt(129) * pi * prt) * sqrt( 13 * log(abs(Rh(:, 2))) + 7 * log(abs(Rh(:, 3))) - 3 * log(abs(Rh(:, 4))) - 17 * log(abs(Rh(:, 5))) );
    Wv = lambda/(4 * sqrt(129) * pi * prt) * sqrt( 13 * log(abs(Rv(:, 2))) + 7 * log(abs(Rv(:, 3))) - 3 * log(abs(Rv(:, 4))) - 17 * log(abs(Rv(:, 5))) );
end

Wh = real(Wh);
Wv = real(Wv);

if N == 2
    D =  abs(Rh(:, 2)) .^ (4 / 3) .* abs(Rv(:, 3)).^(1 / 3) ./ ...
        (abs(Rv(:, 2)) .^ (4 / 3) .* abs(Rh(:, 3)).^(1 / 3));
elseif N == 3
    D =  abs(Rh(:, 2)) .^ (6 / 7) .* abs(Rh(:, 3)) .^ (3 / 7) .* abs(Rv(:, 4)).^(2 / 7) ./ ...
        (abs(Rh(:, 4)) .^ (2 / 7) .* abs(Rv(:, 2)) .^ (6 / 7) .* abs(Rv(:, 3)).^(3 / 7));
elseif N == 4
    D =  abs(Rh(:, 2)) .^ (54 / 86) .* abs(Rh(:, 3)) .^ (39 / 86) .* abs(Rh(:, 4)) .^ (14 / 86) .* abs(Rv(:, 5)) .^ (21 / 86) ./ ...
        (abs(Rh(:, 5)) .^ (21 / 86) .* abs(Rv(:, 2)) .^ (54 / 86) .* abs(Rv(:, 3)) .^ (39 / 86) .* abs(Rv(:, 4)) .^ (14 / 86));
end
D = 10 * log10(D);

P = -angle(C(:, N + 1));

if N == 2
    R = gC .* (abs(Rh(:, 3)) .* abs(Rv(:, 3))) .^ (1 / 6) ./ ...
              (abs(Rh(:, 2)) .* abs(Rv(:, 2))) .^ (2 / 3);
elseif N == 3
    R = gC .* (abs(Rh(:, 4)) .* abs(Rv(:, 4))) .^ (1 / 7) ./ ...
            ( (abs(Rh(:, 2)) .* abs(Rv(:, 2))) .^ (3 / 7) .* (abs(Rh(:, 3)) .* abs(Rv(:, 3))) .^ (3 / 14) );
elseif N == 4
    R = gC .* (abs(Rh(:, 5)) .* abs(Rv(:, 5))) .^ (21 / 172) ./ ...
            ( (abs(Rh(:, 2)) .* abs(Rv(:, 2))) .^ (27 / 86) .* (abs(Rh(:, 3)) .* abs(Rv(:, 3))) .^ (39 / 172) .* (abs(Rh(:, 4)) .* abs(Rv(:, 4))) .^ (7 / 86) );
end

fprintf('Sh =\n')
disp(Sh.')

fprintf('Vh =\n')
disp(Vh.')

fprintf('Wh =\n')
disp(Wh.')

fprintf('Sv =\n')
disp(Sv.')

fprintf('Vv =\n')
disp(Vv.')

fprintf('Wv =\n')
disp(Wv.')

fprintf('\n')

fprintf('D =\n')
disp(D.')

fprintf('P =\n')
disp(P.')

fprintf('R =\n')
disp(R.')
