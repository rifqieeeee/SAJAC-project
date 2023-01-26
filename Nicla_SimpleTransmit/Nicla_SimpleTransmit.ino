#include "Nicla_System.h"
#include "Arduino_BHY2.h"
#include <ArduinoBLE.h>

#define BLE_SENSE_UUID(val) ("19b10000-" val "-537e-4f6c-d104768a1214")

BLEService service(BLE_SENSE_UUID("0000"));
//BLEUnsignedIntCharacteristic temperatureCharacteristic(BLE_SENSE_UUID("2001"), BLERead);
BLEUnsignedIntCharacteristic temperatureCharacteristic(BLE_SENSE_UUID("2001"), BLERead | BLENotify);


// String to calculate the local and device name
String name;

Sensor temperature(SENSOR_ID_TEMP);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  Serial.println("Start");

  nicla::begin();
  nicla::leds.begin();
  nicla::leds.setColor(blue);

  //Sensors initialization
  BHY2.begin(NICLA_STANDALONE);
  temperature.begin();

  if (!BLE.begin()){
    Serial.println("Failed to initialized BLE!");

    while (1)
      ;
  }

  String address = BLE.address();

  Serial.print("address = ");
  Serial.println(address);

  address.toUpperCase();

  name = "NiclaSenseME-";
  name += address[address.length() - 5];
  name += address[address.length() - 4];
  name += address[address.length() - 2];
  name += address[address.length() - 1];

  Serial.print("name = ");
  Serial.println(name);

  BLE.setLocalName(name.c_str());
  BLE.setDeviceName(name.c_str());
  BLE.setAdvertisedService(service);

  // Add all the previously defined Characteristics
  service.addCharacteristic(temperatureCharacteristic);

  // Disconnect event handler
  BLE.setEventHandler(BLEDisconnected, blePeripheralDisconnectHandler);
  
  // Sensors event handlers
  temperatureCharacteristic.setEventHandler(BLERead, onTemperatureCharacteristicRead);

  BLE.addService(service);
  BLE.advertise();
  
}

void loop() {
  // put your main code here, to run repeatedly:
  while (BLE.connected()){
    BHY2.update();
    nicla::leds.setColor(green);

    if (temperatureCharacteristic.subscribed()){
      int temperatureValue = 26;
      temperatureCharacteristic.writeValue(temperatureValue);
    }
  }
}

void blePeripheralDisconnectHandler(BLEDevice central){
  nicla::leds.setColor(red);
}

void onTemperatureCharacteristicRead(BLEDevice central, BLECharacteristic characteristic){
  //String temperatureValue = temperature.value();
  int temperatureValue = 26;
  temperatureCharacteristic.writeValue(temperatureValue);
}
