#include <ArduinoBLE.h>
#include <U8g2lib.h>
 
#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif
 
U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, /* clock=*/ SCL, /* data=*/ SDA, /* reset=*/ U8X8_PIN_NONE);    //Low spped I2C
 

#define BLE_UUID_TEST_SERVICE               "19b10000-0000-537e-4f6c-d104768a1214"
#define BLE_UUID_FILE_NAME                  "19b10000-3001-537e-4f6c-d104768a1214"

BLEService testService( BLE_UUID_TEST_SERVICE );
BLEStringCharacteristic fileNameCharacteristic( BLE_UUID_FILE_NAME, BLERead | BLEWrite, 20 );

String fileName = "";


void setup()
{
  Serial.begin( 9600 );
  while ( !Serial );

  BLE.begin();
  u8g2.begin();
  
  // set advertised local name and service UUID:
  BLE.setDeviceName( "Arduino Nano 33 BLE" );
  BLE.setLocalName( "Arduino Nano 33 BLE" );
  BLE.setAdvertisedService( testService );

  // BLE add characteristics
  testService.addCharacteristic( fileNameCharacteristic );

  // add service
  BLE.addService( testService );

  // set the initial value for the characeristic:
  fileNameCharacteristic.writeValue( fileName );

  // start advertising
  BLE.advertise();

}

void loop()
{
  // listen for BLE peripherals to connect:
  BLEDevice central = BLE.central();

  if ( central )
  {
    Serial.print( "Connected to central: " );
    Serial.println( central.address() );

    while ( central.connected() )
    {
      if ( fileNameCharacteristic.written() )
      {
        fileName = fileNameCharacteristic.value();
        Serial.print( "New file name: " );
        Serial.println( fileName );
        if (fileName = "Emergency!!"){
          u8g2.clearBuffer();                   // clear the internal memory
          u8g2.setFont(u8g2_font_ncenB08_tr);   // choose a suitable font
          u8g2.drawStr(0,15,"Checkpoint1");
          u8g2.drawStr(0,30,"Status : Danger");
          u8g2.drawStr(0,45,"S O S");
          u8g2.drawStr(0,60,"Calling guards . . .");
          u8g2.drawStr(0,30,"Checkpoint1");
          u8g2.sendBuffer(); 
          delay(5000);
          u8g2.clearBuffer();
          u8g2.sendBuffer(); 
        }
      }
    }

    Serial.print( F( "Disconnected from central: " ) );
    Serial.println( central.address() );
  }
}
