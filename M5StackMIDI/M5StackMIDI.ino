
// define must ahead #include <M5Stack.h>
#define M5STACK_MPU6886

#include <M5Stack.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

// Bluetooth device name
#define DEVICE_NAME "M5Stack_BLE_MIDI"

// BLE MIDI GATT service and characteristic
#define MIDI_SERVICE_UUID        "03b80e5a-ede8-4b33-a751-6ce34ec4c700"
#define MIDI_CHARACTERISTIC_UUID "7772e5db-3868-4112-a1a9-f2669d106bf3"

// BLE GATT server
BLEServer *pServer;

// BLE GATT characteristic
BLECharacteristic *pCharacteristic;

// BLE connection state
bool isConnected = false;

// Send MIDI Packet
uint8_t midiPacket[] = {
  0x80,  // header
  0x80,  // timestamp
  0x00,  // status
  0x3c,  // 60
  0x00   // velocity
};

// BLE GATT Serveri callbacks
class cbServer: public BLEServerCallbacks {
    void onConnect(BLEServer *pServer) {
      isConnected = true;
      M5.Lcd.clear();
      M5.Lcd.drawCentreString(" BLE Connected  ", 160, 120, 4);
    };

    void onDisconnect(BLEServer *pServer) {
      isConnected = false;
      M5.Lcd.clear();
      M5.Lcd.drawCentreString(DEVICE_NAME, 160, 120, 4);
    }
};

void setup() {
  // Initialize M5Stack
  M5.begin();
  M5.Power.begin();
  M5.IMU.Init();

  // BLE Initialize
  BLEDevice::init(DEVICE_NAME);

  // Create BLE GATT server
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new cbServer());
  BLEDevice::setEncryptionLevel((esp_ble_sec_act_t)ESP_LE_AUTH_REQ_SC_BOND);

  // Create BLE GATT service
  BLEService* pService = pServer->createService(BLEUUID(MIDI_SERVICE_UUID));

  // Create BLE GATT characteristic
  pCharacteristic = pService->createCharacteristic(
                      BLEUUID(MIDI_CHARACTERISTIC_UUID),
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_NOTIFY |
                      BLECharacteristic::PROPERTY_WRITE_NR
                    );
  pCharacteristic->setAccessPermissions(ESP_GATT_PERM_READ_ENCRYPTED | ESP_GATT_PERM_WRITE_ENCRYPTED);

  // Set CCCD
  pCharacteristic->addDescriptor(new BLE2902());

  // Start BLE GATT service
  pService->start();

  // Start BLE advertising
  BLESecurity* pSecurity = new BLESecurity();
  pSecurity->setAuthenticationMode(ESP_LE_AUTH_REQ_SC_BOND);
  pSecurity->setCapability(ESP_IO_CAP_NONE);
  pSecurity->setInitEncryptionKey(ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK);
  pServer->getAdvertising()->addServiceUUID(MIDI_SERVICE_UUID);
  pServer->getAdvertising()->start();

  // Show a device name on M5Stack display
  M5.Lcd.drawCentreString(DEVICE_NAME, 160, 120, 4);
}

void loop() {
  M5.update();

  if (!isConnected) {
    // Do nothing
    return;
  }

  if (M5.BtnA.wasPressed()) {
    // note down
    midiPacket[2] = 0x90; // note down, channel 0
    midiPacket[3] = 0x3C; //
    midiPacket[4] = 127;  // velocity
    pCharacteristic->setValue(midiPacket, 5); // packet, length in bytes
    pCharacteristic->notify();
  }

  if (M5.BtnA.wasReleased()) {
    // note up
    midiPacket[2] = 0x80; // note up, channel 0
    midiPacket[3] = 0x3C; //
    midiPacket[4] = 0;    // velocity
    pCharacteristic->setValue(midiPacket, 5); // packet, length in bytes)
    pCharacteristic->notify();
  }

  if (M5.BtnB.wasPressed()) {
    // note down
    midiPacket[2] = 0x90; // note down, channel 0
    midiPacket[3] = 0x3F; //
    midiPacket[4] = 127;  // velocity
    pCharacteristic->setValue(midiPacket, 5); // packet, length in bytes
    pCharacteristic->notify();
  }

  if (M5.BtnB.wasReleased()) {
    // note up
    midiPacket[2] = 0x80; // note up, channel 0
    midiPacket[3] = 0x3F; //
    midiPacket[4] = 0;    // velocity
    pCharacteristic->setValue(midiPacket, 5); // packet, length in bytes)
    pCharacteristic->notify();
  }

  if (M5.BtnC.wasPressed()) {
    // note down
    midiPacket[2] = 0x90; // note down, channel 0
    midiPacket[3] = 0x43; //
    midiPacket[4] = 127;  // velocity
    pCharacteristic->setValue(midiPacket, 5); // packet, length in bytes
    pCharacteristic->notify();
  }

  if (M5.BtnC.wasReleased()) {
    // note up
    midiPacket[2] = 0x80; // note up, channel 0
    midiPacket[3] = 0x43; //
    midiPacket[4] = 0;    // velocity
    pCharacteristic->setValue(midiPacket, 5); // packet, length in bytes)
    pCharacteristic->notify();
  }

  if (M5.BtnA.isPressed()) {
    // Get sensor data
    float pitch = 0.0F;
    float roll = 0.0F;
    float yaw = 0.0F;
    M5.IMU.getAhrsData(&pitch, &roll, &yaw);

    // Mapping roll value
    int ccValue = map((int)roll, -10, 110, 0, 127);

    // Constrain range
    ccValue = constrain(ccValue, 0, 127);

    // Send MIDI CC
    Serial.println((int)ccValue, DEC);
    midiPacket[2] = 0xB0; // Control change, channel 0
    midiPacket[3] = 0x07; // CC num
    midiPacket[4] = ccValue; // CC value
    pCharacteristic->setValue(midiPacket, 5); // packet, length in bytes)
    pCharacteristic->notify();
  }
  delay(1);
}
