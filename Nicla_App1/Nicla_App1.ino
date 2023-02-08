#include "Nicla_System.h"
#include "Arduino_BHY2.h"
#include <ArduinoBLE.h>
#include <Sajac_inferencing.h>

static const float features[25] = {};
int user_status;

#define BLE_SENSE_UUID(val) ("19b10000-" val "-537e-4f6c-d104768a1214")

BLEService service(BLE_SENSE_UUID("0000"));
BLEUnsignedIntCharacteristic idCharacteristic(BLE_SENSE_UUID("1001"), BLERead | BLENotify);
BLEUnsignedIntCharacteristic temperatureCharacteristic(BLE_SENSE_UUID("2001"), BLERead | BLENotify);
BLEUnsignedIntCharacteristic humidityCharacteristic(BLE_SENSE_UUID("3001"), BLERead | BLENotify);
BLEUnsignedIntCharacteristic pressureCharacteristic(BLE_SENSE_UUID("4001"), BLERead | BLENotify);
BLEUnsignedIntCharacteristic gasCharacteristic(BLE_SENSE_UUID("5001"), BLERead | BLENotify);
BLEUnsignedIntCharacteristic airqualityCharacteristic(BLE_SENSE_UUID("6001"), BLERead | BLENotify);
BLEUnsignedIntCharacteristic statusCharacteristic(BLE_SENSE_UUID("7001"), BLERead | BLENotify);

//BLEService service("1000");
//BLEUnsignedIntCharacteristic idCharacteristic("1100", BLERead | BLENotify);
//BLEUnsignedIntCharacteristic temperatureCharacteristic("1200", BLERead | BLENotify);
//BLEUnsignedIntCharacteristic humidityCharacteristic("1300", BLERead | BLENotify);
//BLEUnsignedIntCharacteristic pressureCharacteristic("1400", BLERead | BLENotify);
//BLEUnsignedIntCharacteristic gasCharacteristic("1500", BLERead | BLENotify);
//BLEUnsignedIntCharacteristic airqualityCharacteristic("1600", BLERead | BLENotify);
//BLEUnsignedIntCharacteristic statusCharacteristic("1700", BLERead | BLENotify);

// String to calculate the local and device name
String name;

Sensor temperature(SENSOR_ID_TEMP);
Sensor humidity(SENSOR_ID_HUM);
Sensor pressure(SENSOR_ID_BARO);
Sensor gas(SENSOR_ID_GAS);
SensorBSEC bsec(SENSOR_ID_BSEC);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  Serial.println("Start");

  nicla::begin();
  nicla::enableCharge(100);
  nicla::leds.begin();
  nicla::leds.setColor(red);

  //Sensors initialization
  BHY2.begin(NICLA_STANDALONE);
  temperature.begin();
  humidity.begin();
  pressure.begin();
  gas.begin();
  bsec.begin();

  if (!BLE.begin()){
    Serial.println("Failed to initialized BLE!");

    while (1)
      ;
  }

  String address = BLE.address();

  Serial.print("address = ");
  Serial.println(address);

  address.toUpperCase();

  name = "Sajac Sensor-";
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
  service.addCharacteristic(idCharacteristic);
  service.addCharacteristic(temperatureCharacteristic);
  service.addCharacteristic(humidityCharacteristic);
  service.addCharacteristic(pressureCharacteristic);
  service.addCharacteristic(gasCharacteristic);
  service.addCharacteristic(airqualityCharacteristic);
  service.addCharacteristic(statusCharacteristic);

  // Disconnect event handler
  BLE.setEventHandler(BLEDisconnected, blePeripheralDisconnectHandler);
  
  // Sensors event handlers
  idCharacteristic.setEventHandler(BLERead, onIdCharacteristicRead);
  temperatureCharacteristic.setEventHandler(BLERead, onTemperatureCharacteristicRead);
  humidityCharacteristic.setEventHandler(BLERead, onHumidityCharacteristicRead);
  pressureCharacteristic.setEventHandler(BLERead, onPressureCharacteristicRead);
  gasCharacteristic.setEventHandler(BLERead, onGasCharacteristicRead);
  airqualityCharacteristic.setEventHandler(BLERead, onAirqualityCharacteristicRead);
  statusCharacteristic.setEventHandler(BLERead, onStatusCharacteristicRead);

  BLE.addService(service);
  BLE.advertise();
  
}

int raw_feature_get_data(size_t offset, size_t length, float *out_ptr) {
  float features[25];
  int temperatureValue = temperature.value();
  int humidityValue = humidity.value();
  int pressureValue = pressure.value();
  int gasValue = gas.value();
  int airquality = bsec.iaq();
  for (byte i = 0; i < 25; i = i + 5){
    features[i]=temperatureValue;
    features[i+1]=humidityValue;
    features[i+2]=pressureValue;
    features[i+3]=gasValue;
    features[i+4]=airquality;
    delay(200);
  }
  memcpy(out_ptr, features + offset, length * sizeof(float));
  return 0;
}

void print_inference_result(ei_impulse_result_t result);

void ml(){
  //    ei_printf("Edge Impulse standalone inferencing (Arduino)\n");

  if (sizeof(features) / sizeof(float) != EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE) {
      ei_printf("The size of your 'features' array is not correct. Expected %lu items, but had %lu\n",
          EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE, sizeof(features) / sizeof(float));
      delay(1000);
      return;
  }

  ei_impulse_result_t result = { 0 };

  // the features are stored into flash, and we don't want to load everything into RAM
  signal_t features_signal;
  features_signal.total_length = sizeof(features) / sizeof(features[0]);
  features_signal.get_data = &raw_feature_get_data;

  // invoke the impulse
  EI_IMPULSE_ERROR res = run_classifier(&features_signal, &result, false /* debug */);
  if (res != EI_IMPULSE_OK) {
      ei_printf("ERR: Failed to run classifier (%d)\n", res);
      return;
  }

  // print inference return code
//    ei_printf("run_classifier returned: %d\r\n", res);
  print_inference_result(result);
  Serial.println(user_status);
}

void loop() {
  // put your main code here, to run repeatedly:
  while (BLE.connected()){
    ml();
    BHY2.update();
    nicla::leds.setColor(green);

    if (idCharacteristic.subscribed()){
      int idValue = 23;
      Serial.println(idValue);
      idCharacteristic.writeValue(idValue);
    }
    
    if (temperatureCharacteristic.subscribed()){
      int temperatureValue = temperature.value();
      Serial.println(temperatureValue);
      temperatureCharacteristic.writeValue(temperatureValue);
    }

    if (humidityCharacteristic.subscribed()){
      int humidityValue = humidity.value();
      Serial.println(humidityValue);
      humidityCharacteristic.writeValue(humidityValue);
    }

    if (pressureCharacteristic.subscribed()){
      int pressureValue = pressure.value();
      Serial.println(pressureValue);
      pressureCharacteristic.writeValue(pressureValue);
    }

    if (gasCharacteristic.subscribed()){
      int gasValue = gas.value();
      Serial.println(gasValue);
      gasCharacteristic.writeValue(gasValue);
    }

    if (airqualityCharacteristic.subscribed()){
      int airquality = bsec.iaq();
      Serial.println(airquality);
      airqualityCharacteristic.writeValue(airquality);
    }

    if (statusCharacteristic.subscribed()){
      int statusValue = user_status;
      Serial.println(statusValue);
      statusCharacteristic.writeValue(statusValue);
    }
  }
}

void blePeripheralDisconnectHandler(BLEDevice central){
  nicla::leds.setColor(red);
}

void onIdCharacteristicRead(BLEDevice central, BLECharacteristic characteristic){
  int idValue = 23;
  idCharacteristic.writeValue(idValue);
}

void onTemperatureCharacteristicRead(BLEDevice central, BLECharacteristic characteristic){
  int temperatureValue = temperature.value();
  temperatureCharacteristic.writeValue(temperatureValue);
}

void onHumidityCharacteristicRead(BLEDevice central, BLECharacteristic characteristic){
  int humidityValue = humidity.value();
  humidityCharacteristic.writeValue(humidityValue);
}

void onPressureCharacteristicRead(BLEDevice central, BLECharacteristic characteristic){
  int pressureValue = pressure.value();
  pressureCharacteristic.writeValue(pressureValue);
}

void onGasCharacteristicRead(BLEDevice central, BLECharacteristic characteristic){
  int gasValue = gas.value();
  gasCharacteristic.writeValue(gasValue);
}

void onAirqualityCharacteristicRead(BLEDevice central, BLECharacteristic characteristic){
  int airquality = bsec.iaq();
  airqualityCharacteristic.writeValue(airquality);
}

void onStatusCharacteristicRead(BLEDevice central, BLECharacteristic characteristic){
  int statusValue = user_status;
  statusCharacteristic.writeValue(statusValue);
}

void print_inference_result(ei_impulse_result_t result) {

    // Print how long it took to perform inference
//    ei_printf("Timing: DSP %d ms, inference %d ms, anomaly %d ms\r\n",
//            result.timing.dsp,
//            result.timing.classification,
//            result.timing.anomaly);

    // Print the prediction results (object detection)
#if EI_CLASSIFIER_OBJECT_DETECTION == 1
    ei_printf("Object detection bounding boxes:\r\n");
    for (uint32_t i = 0; i < result.bounding_boxes_count; i++) {
        ei_impulse_result_bounding_box_t bb = result.bounding_boxes[i];
        if (bb.value == 0) {
            continue;
        }
        ei_printf("  %s (%f) [ x: %u, y: %u, width: %u, height: %u ]\r\n",
                bb.label,
                bb.value,
                bb.x,
                bb.y,
                bb.width,
                bb.height);
    }

    // Print the prediction results (classification)
#else
//    ei_printf("Predictions:\r\n");
    for (uint16_t i = 0; i < EI_CLASSIFIER_LABEL_COUNT; i++) {
//        ei_printf("  %s: ", ei_classifier_inferencing_categories[i]);
//        ei_printf("%.5f\r\n", result.classification[i].value);
    }
    if (result.classification[0].value > result.classification[1].value){
      user_status = 0;
    }else user_status = 1;
#endif

    // Print anomaly result (if it exists)
#if EI_CLASSIFIER_HAS_ANOMALY == 1
    ei_printf("Anomaly prediction: %.3f\r\n", result.anomaly);
#endif

}
