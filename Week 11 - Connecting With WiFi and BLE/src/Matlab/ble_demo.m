% Identify esp32 by name specified in C++
device = ble("MEC4980_esp32_ble"); 

% These are universally unique identifiers (UUIDs) for the service and 
% telemetry characteristic. 
% They must match the UUIDs in the C++ code.
serviceUUID = "ea841108-52cc-4fca-9a60-c8e90bf5bb23"; 
telemetryUUID = "ea841109-52cc-4fca-9a60-c8e90bf5bb23";
pixelCommandUUID = "ea84110a-52cc-4fca-9a60-c8e90bf5bb23";
telemetry = characteristic(device, serviceUUID, telemetryUUID); % Create a characteristic object to read data from esp32
pixelControl = characteristic(device, serviceUUID, pixelCommandUUID);

% Callback function to process data
telemetry.DataAvailableFcn = @receiveTelemetry; 

% Subscribe to esp32 ble output
subscribe(telemetry); 

% Simple function to receive and display data from esp32
function receiveTelemetry(source, ~)
    bytes = read(source, "oldest");
    jsonText = native2unicode(uint8(bytes(:).'), "UTF-8");
    sample = jsondecode(jsonText);

    fprintf("Sample %d: buttons = %s\n", sample.n, sample.v);
end


% Copy this line into the Command Window; ledState of 0 (off), 1 (red),
% 2 (green), or 3 (blue) 
% ledState = 1; write(pixelControl, uint8(ledState), "uint8", "withresponse");

% To end communication:
% unsubscribe(telemetry); clear pixelControl telemetry device