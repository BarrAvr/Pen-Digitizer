#include <Arduino.h>

#include "SparkFunLSM6DSO.h" // Gyroscope
#include "Wire.h"

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>
#define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

#define MODE_NONE_DOWN 0
#define MODE_B1_DOWN 1
#define MODE_B2_DOWN 2

bool btn1down = false;
bool btn2down = false;

float x;
float y;
float z;
int mode;

LSM6DSO myIMU; //Default constructor is I2C, addr 0x6B
int stepCount = 0;
BLECharacteristic *pCharacteristic;
bool deviceConnected = false;

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      Serial.println("Device connected");
      deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
      Serial.println("Device disconnected");
      deviceConnected = false;
    }
};

class MyCallbacks: public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    std::string value = pCharacteristic->getValue();
    
    if (value.length() > 0) {
      Serial.println("*********");
      Serial.print("New value: ");
      for (int i = 0; i < value.length(); i++)
        Serial.print(value[i]);
      Serial.println();
      Serial.println("*********");
    }
  }
};

void setup() {
  Serial.begin(115200);
  delay(500);

  pinMode(0, INPUT_PULLUP);
  pinMode(35, INPUT_PULLUP);

  // Connecting to gyroscope
  Wire.begin();
  delay(100);
  myIMU = LSM6DSO();
  if( myIMU.begin() ) {
    Serial.println("Gyroscope ready.");
  } else { 
    Serial.println("Could not connect to IMU.");
    Serial.println("Freezing");
  }
  if( myIMU.initialize(BASIC_SETTINGS) ) {
    Serial.println("Loaded Settings.");
  }

  // Starting BLE  
  Serial.println("Starting BLE work!");

  Serial.println("1- Download and install an BLE scanner app in your phone");
  Serial.println("2- Scan for BLE devices in the app");
  Serial.println("3- Connect to MyESP32");
  Serial.println("4- Go to CUSTOM CHARACTERISTIC in CUSTOM SERVICE and write something");
  Serial.println("5- See the magic =)");

  BLEDevice::init("CS147");
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  BLEService *pService = pServer->createService(SERVICE_UUID);
  pCharacteristic = pService->createCharacteristic(
                                            CHARACTERISTIC_UUID,
                                            BLECharacteristic::PROPERTY_READ |
                                            BLECharacteristic::PROPERTY_WRITE |
                                            BLECharacteristic::PROPERTY_NOTIFY |
                                            BLECharacteristic::PROPERTY_INDICATE
                                          );

  pCharacteristic->addDescriptor(new BLE2902()); // Added newly

  pCharacteristic->setCallbacks(new MyCallbacks());

  pCharacteristic->setValue("Server Example – CS147"); //What gets transfered
  pService->start();
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x0);
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();
  Serial.println("Characteristic defined! Now you can read it in your phone!");
}

void loop() {
  // Get and print all gyroscope data
  // Serial.print("\nAccelerometer:\n");
  // Serial.print(" X = ");
  // Serial.println(myIMU.readFloatAccelX(), 3);
  // Serial.print(" Y = ");
  // Serial.println(myIMU.readFloatAccelY(), 3);
  // Serial.print(" Z = ");
  // Serial.println(myIMU.readFloatAccelZ(), 3);
  // Serial.print("\nGyroscope:\n");
  
  // float x = myIMU.readFloatAccelX();
  // float y = myIMU.readFloatAccelY();
  // float z = myIMU.readFloatAccelZ();
  // xyz[0] = x;
  // xyz[1] = y;
  // xyz[2] = z;
  if (digitalRead(0) == 0) {
    btn1down = true;
  } else {
    btn1down = false;
  }
  if (digitalRead(35) == 0) {
    btn2down = true;
  } else {
    btn2down = false;
  }

  x = myIMU.readFloatGyroX();
  y = myIMU.readFloatGyroY();
  z = myIMU.readFloatGyroZ();
  if (btn1down) {
    mode = MODE_B1_DOWN;
  } else if (btn2down) {
    mode = MODE_B2_DOWN;
  } else {
    mode = MODE_NONE_DOWN;
  }
  std::string r = std::string(String(x).c_str()) + "," + std::string(String(y).c_str()) + "," + std::string(String(z).c_str()) + "," + std::string(String(mode).c_str());
  pCharacteristic->setValue(r);
  // pCharacteristic->setValue(xyz, 3);
  pCharacteristic->notify();
  delay(100); // reading every 50 ms is too fast for the chip?

  // Serial.print(" Y = ");
  // Serial.println(myIMU.readFloatGyroY(), 3);
  // Serial.print(" Z = ");
  // Serial.println(myIMU.readFloatGyroZ(), 3);

  // Serial.print("\nThermometer:\n");
  // Serial.print(" Degrees F = ");
  // Serial.println(myIMU.readTempF(), 3);
}