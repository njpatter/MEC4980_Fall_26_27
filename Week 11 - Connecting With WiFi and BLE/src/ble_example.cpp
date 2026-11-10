#include <Arduino.h>
#include <ArduinoJson.h>
#include <BLE2902.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <atomic>

uint32_t lastSampleMs = 0;
uint16_t sequence = 0;

constexpr char deviceName[] = "MEC4980_esp32_ble";
constexpr char serviceUuid[] = "ea841108-52cc-4fca-9a60-c8e90bf5bb23";
constexpr char telemetryUuid[] = "ea841109-52cc-4fca-9a60-c8e90bf5bb23";
constexpr char pixelCommandUuid[] = "ea84110a-52cc-4fca-9a60-c8e90bf5bb23";
constexpr uint32_t samplePeriodMs = 200;
constexpr size_t maxPayloadBytes = 20;

std::atomic<bool> connected{false};
std::atomic<uint8_t> pixelCommand{0};
BLECharacteristic* telemetry = nullptr;
BLE2902* clientConfiguration = nullptr;

class PixelCallbacks : public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic* characteristic) override {
        const std::string value = characteristic->getValue();
        if (value.size() != 1 || static_cast<uint8_t>(value[0]) > 3) {
            Serial.println("Invalid NeoPixel command: expected one byte from 0 to 3");
            return;
        }
        pixelCommand.store(static_cast<uint8_t>(value[0]));
    }
};

class ConnectionCallbacks : public BLEServerCallbacks {
    void onConnect(BLEServer*) override {
        clientConfiguration->setNotifications(false);
        connected.store(true);
    }

    void onDisconnect(BLEServer*) override {
        connected.store(false);
        BLEDevice::startAdvertising();
    }
};

ConnectionCallbacks connectionCallbacks;
PixelCallbacks pixelCallbacks;


void setup() {
    Serial.begin(115200);
    pinMode(1, INPUT_PULLDOWN);
    pinMode(2, INPUT_PULLDOWN);  
    pinMode(NEOPIXEL_POWER, OUTPUT);
    digitalWrite(NEOPIXEL_POWER, NEOPIXEL_POWER_ON);
    neopixelWrite(PIN_NEOPIXEL, 0, 0, 0);

    BLEDevice::init(deviceName);
    auto* server = BLEDevice::createServer();
    server->setCallbacks(&connectionCallbacks);

    auto* service = server->createService(serviceUuid);
    telemetry = service->createCharacteristic(
        telemetryUuid, BLECharacteristic::PROPERTY_NOTIFY);
    auto* pixelControl = service->createCharacteristic(
        pixelCommandUuid, BLECharacteristic::PROPERTY_WRITE);
    pixelControl->setCallbacks(&pixelCallbacks);

    // The central enables notifications by writing this descriptor.
    clientConfiguration = new BLE2902();
    telemetry->addDescriptor(clientConfiguration);
    service->start();

    auto* advertising = BLEDevice::getAdvertising();
    advertising->addServiceUUID(serviceUuid);
    advertising->setScanResponse(true);
    BLEDevice::startAdvertising();
    Serial.print("Advertising as ");
    Serial.println(deviceName);
}

void loop() {
    static uint8_t displayedCommand = 0;
    const uint8_t command = pixelCommand.load();
    if (command != displayedCommand) {
        neopixelWrite(PIN_NEOPIXEL,
                      command == 1 ? RGB_BRIGHTNESS : 0,
                      command == 2 ? RGB_BRIGHTNESS : 0,
                      command == 3 ? RGB_BRIGHTNESS : 0);
        displayedCommand = command;
    }

    const uint32_t now = millis();
    // Only notify once the central has subscribed (e.g. MATLAB's subscribe()).
    if (!connected.load() || !clientConfiguration->getNotifications() ||
        now - lastSampleMs < samplePeriodMs) {
        delay(5);
        return;
    }
    lastSampleMs = now;

    JsonDocument sample;
    sample["n"] = sequence;
    sample["v"] = String(int(digitalRead(1))) + ", "+ String(int(digitalRead(2)));
    ++sequence;

    // A complete JSON object must fit in one default-MTU notification.
    char payload[maxPayloadBytes + 1];
    if (sample.overflowed() || measureJson(sample) > maxPayloadBytes) {
        Serial.println("Skipping sample: JSON exceeds the notification budget");
        return;
    }

    const size_t size = serializeJson(sample, payload, sizeof(payload));
    telemetry->setValue(reinterpret_cast<uint8_t*>(payload), size);
    telemetry->notify();
}
