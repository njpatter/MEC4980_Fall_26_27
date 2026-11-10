function [device, telemetry] = start_feather_ble(identifier) 

if nargin == 0
    identifier = "MEC4980_esp32_ble";
end

serviceUuid = "ea841108-52cc-4fca-9a60-c8e90bf5bb23";
telemetryUuid = "ea841109-52cc-4fca-9a60-c8e90bf5bb23";

device = ble(identifier);
telemetry = characteristic(device, serviceUuid, telemetryUuid);
telemetry.DataAvailableFcn = @onTelemetry;
try
    subscribe(telemetry);
catch exception
    telemetry.DataAvailableFcn = [];
    rethrow(exception);
end
end

function onTelemetry(source, ~)
% Read the oldest queued notification from MATLAB's BLE buffer.
try
    [bytes, timestamp] = read(source, "oldest");
    if isempty(bytes)
        return
    end

    json = native2unicode(uint8(bytes(:).'), "UTF-8");
    sample = jsondecode(json);
    fprintf('%s  n=%5d  buttons=%s\n', char(string(timestamp)), sample.n, sample.v);
catch exception
    warning('FeatherBLE:Notification', ...
        'Could not process a notification: %s', exception.message);
end
end
