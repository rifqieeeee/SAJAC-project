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
  if (millis() - lastCheck >= 200) {
    lastCheck = millis();
    Serial.print(int(temperature.value()));
    Serial.print(",");
    Serial.print(int(humidity.value()));
    Serial.print(",");
    Serial.print(int(pressure.value()));
    Serial.print(",");
    Serial.print(int(gas.value()));
    Serial.print(",");
    Serial.println(int(bsec.iaq()));
    
  }
}
