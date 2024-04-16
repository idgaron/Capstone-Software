S = serialport('/dev/cu.wchusbserial57710030221',115200);
values = [];
while true
    if S.NumBytesAvailable > 0
        data = readline(S);
        values = [values str2double(split(data,','))];
    end
end v