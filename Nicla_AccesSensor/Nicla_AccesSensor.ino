#include "Nicla_System.h"
#include "Arduino_BHY2.h"

Sensor temperature(SENSOR_ID_TEMP);
Sensor humidity(SENSOR_ID_HUM);
Sensor pressure(SENSOR_ID_BARO);
Sensor gas(SENSOR_ID_GAS);
SensorBSEC bsec(SENSOR_ID_BSEC);


void setup(){
  nicla::begin();
  nicla::enableCharge(100);
  
  Serial.begin(115200);
  BHY2.begin();
  temperature.begin();
  humidity.begin();
  pressure.begin();
  gas.begin();
  bsec.begin();
}

void loop(){
  static auto lastCheck= millis();
  BHY2.update();

  // Check sensor values every second  
  if (millis() - lastCheck >= 1000) {
    lastCheck = millis();
    Serial.println(String("temperature: ") + String(int(temperature.value())));
    Serial.println(String("humidity: ") + String(int(humidity.value())));
    Serial.println(String("pressure: ") + String(int(pressure.value())));
    Serial.println(String("gas: ") + String(int(gas.value())));
    Serial.println(String("air quality: ") + String(int(bsec.iaq())));
    Serial.println(String(""));
  }
}
