#include <ArduinoBLE.h>
#include <Arduino.h>
#include <HardwareSerial.h>
#include <Wire.h>
#include <U8g2lib.h>
 
#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif
 
U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, /* clock=*/ SCL, /* data=*/ SDA, /* reset=*/ U8X8_PIN_NONE);    //Low spped I2C
HardwareSerial e5(0);

#define BLE_UUID_TEST_SERVICE               "19b10000-0000-537e-4f6c-d104768a1214"
#define BLE_UUID_FILE_NAME                  "19b10000-1001-537e-4f6c-d104768a1214"

BLEService testService( BLE_UUID_TEST_SERVICE );
BLEStringCharacteristic fileNameCharacteristic( BLE_UUID_FILE_NAME, BLERead | BLEWrite, 20 );

String fileName = "";

int data1=1;
int data2=3;
int data3=5; 

static char recv_buf[512];
static bool is_exist = false;

static int at_send_check_response(char *p_ack, int timeout_ms, char *p_cmd, ...)
{
    int ch = 0;
    int index = 0;
    int startMillis = 0;
    va_list args;
    memset(recv_buf, 0, sizeof(recv_buf));
    va_start(args, p_cmd);
    e5.printf(p_cmd, args);
    Serial.printf(p_cmd, args);
    va_end(args);
    delay(200);
    startMillis = millis();
 
    if (p_ack == NULL)
    {
        return 0;
    }
 
    do
    {
        while (e5.available() > 0)
        {
            ch = e5.read();
            recv_buf[index++] = ch;
            Serial.print((char)ch);
            delay(2);
        }
 
        if (strstr(recv_buf, p_ack) != NULL)
        {
            return 1;
        }
 
    } while (millis() - startMillis < timeout_ms);
    return 0;
}

static int node_send(uint32_t timeout)
{
    static uint16_t count = 0;
    int ret = 0;
    char data[32];
    char cmd[128];
    
    
    
    memset(data, 0, sizeof(data));
    sprintf(data, "%04X,%04X,%04X", data1, data2, data3);
    sprintf(cmd, "AT+TEST=TXLRPKT,\"5345454544%s\"\r\n", data);
    ret = at_send_check_response("TX DONE", 2000, cmd);
    if (ret == 1)
    {
 
        //send Airquality
        Serial.print("Sent successfully!\r\n");
        
    }
    else
    {
        Serial.print("Send failed!\r\n");
    }
    return ret;
}


void setup()
{
  Serial.begin( 9600 );
  e5.begin(9600, SERIAL_8N1, -1, -1);
//  while ( !Serial );

  BLE.begin();
  u8g2.begin();
  
  // set advertised local name and service UUID:
  BLE.setDeviceName( "Sajac Transmitter" );
  BLE.setLocalName( "Sajac Transmitter" );
  BLE.setAdvertisedService( testService );

  // BLE add characteristics
  testService.addCharacteristic( fileNameCharacteristic );

  // add service
  BLE.addService( testService );

  // set the initial value for the characeristic:
  fileNameCharacteristic.writeValue( fileName );

  // start advertising
  BLE.advertise();

  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB08_tr);
  u8g2.drawStr(0,15,"Sajac Transmitter");
  u8g2.drawStr(0,30,"Standby mode");
  u8g2.sendBuffer();

  uint16_t error;
  char errorMessage[256];
  
  Serial.print("ping pong communication!\r\n");

  if (at_send_check_response("+AT: OK", 100, "AT\r\n"))
  {
      is_exist = true;
      at_send_check_response("+MODE: TEST", 1000, "AT+MODE=TEST\r\n");
      at_send_check_response("+TEST: RFCFG", 1000, "AT+TEST=RFCFG,866,SF12,125,12,15,14,ON,OFF,OFF\r\n");
      delay(200);

  }
  else
  {
      is_exist = false;
      Serial.print("No E5 module found.\r\n");

  }
}

void loop()
{
  // listen for BLE peripherals to connect:
  BLEDevice central = BLE.central();

  if ( central )
  {
    Serial.print( "Connected to central: " );
    Serial.println( central.address() );
    
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_ncenB08_tr);
    u8g2.drawStr(0,15,"Connected to App");
    u8g2.sendBuffer();
    delay(1500);
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_ncenB08_tr);
    u8g2.drawStr(0,15,"Please select");
    u8g2.drawStr(0,30,"your current status");
    u8g2.drawStr(0,45,"on Sajac App");
    u8g2.sendBuffer();

    while ( central.connected() )
    {
      if ( fileNameCharacteristic.written() )
      {
        fileName = fileNameCharacteristic.value();
        if ( fileName != ""){
          Serial.print( "New file name: " );
          Serial.println( fileName );
        }
        
        if (fileName == "Safe"){
          u8g2.clearBuffer();                   // clear the internal memory
          u8g2.setFont(u8g2_font_ncenB08_tr);   // choose a suitable font
          u8g2.drawStr(0,15,"Checkpoint1");
          u8g2.drawStr(0,30,"Status : Safe");
          u8g2.drawStr(0,45,"Ready to ");
          u8g2.drawStr(0,60,"Next Checkpoint");
          u8g2.sendBuffer(); 
          delay(2000);

          data1 = 1;
          data2 = 23;
          data3 = 1;

          for(int i=1; i<=2; i++){
            if (is_exist){
              //node_send_then_recv(2000);
              node_send(2000);
              delay(1000);
            }else{
              u8g2.clearBuffer();
              u8g2.setFont(u8g2_font_ncenB08_tr);
              u8g2.drawStr(0,15,"Try Again!");
              u8g2.sendBuffer();
              delay(5000);
            }
          }

          u8g2.clearBuffer();
          u8g2.setFont(u8g2_font_ncenB08_tr);
          u8g2.drawStr(0,15,"Please select");
          u8g2.drawStr(0,30,"your current status");
          u8g2.drawStr(0,45,"on Sajac App");
          u8g2.sendBuffer();

          fileName = "";
        }

        else if (fileName == "Bad"){
          u8g2.clearBuffer();                   // clear the internal memory
          u8g2.setFont(u8g2_font_ncenB08_tr);   // choose a suitable font
          u8g2.drawStr(0,15,"Checkpoint1");
          u8g2.drawStr(0,30,"Status : Bad");
          u8g2.drawStr(0,45,"Please go");
          u8g2.drawStr(0,60,"Back!");
          u8g2.sendBuffer(); 
          delay(1000);

          data1 = 1;
          data2 = 23;
          data3 = 2;

          for(int i=1; i<=2; i++){
            if (is_exist){
              //node_send_then_recv(2000);
              node_send(2000);
              delay(3000);
            }else{
              u8g2.clearBuffer();
              u8g2.setFont(u8g2_font_ncenB08_tr);
              u8g2.drawStr(0,15,"Try Again!");
              u8g2.sendBuffer();
              delay(5000);
            }
          }

          u8g2.clearBuffer();
          u8g2.setFont(u8g2_font_ncenB08_tr);
          u8g2.drawStr(0,15,"Please select");
          u8g2.drawStr(0,30,"your current status");
          u8g2.drawStr(0,45,"on Sajac App");
          u8g2.sendBuffer();

          fileName = "";
        }

        else if (fileName == "Danger"){
          u8g2.clearBuffer();                   // clear the internal memory
          u8g2.setFont(u8g2_font_ncenB08_tr);   // choose a suitable font
          u8g2.drawStr(0,15,"Checkpoint1");
          u8g2.drawStr(0,30,"Status : Danger");
          u8g2.drawStr(0,45,"S O S");
          u8g2.drawStr(0,60,"Calling guards . . .");
          u8g2.sendBuffer(); 
          delay(1000);

          data1 = 1;
          data2 = 23;
          data3 = 3;

          for(int i=1; i<=2; i++){
            if (is_exist){
              //node_send_then_recv(2000);
              node_send(2000);
              delay(3000);
            }else{
              u8g2.clearBuffer();
              u8g2.setFont(u8g2_font_ncenB08_tr);
              u8g2.drawStr(0,15,"Try Again!");
              u8g2.sendBuffer();
              delay(5000);
            }
          }

          u8g2.clearBuffer();
          u8g2.setFont(u8g2_font_ncenB08_tr);
          u8g2.drawStr(0,15,"Please select");
          u8g2.drawStr(0,30,"your current status");
          u8g2.drawStr(0,45,"on Sajac App");
          u8g2.sendBuffer();

          fileName = "";
        }
      }
    }

    Serial.print( F( "Disconnected from central: " ) );
    Serial.println( central.address() );
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_ncenB08_tr);
    u8g2.drawStr(0,15,"Disconnecting . . .");
    u8g2.sendBuffer();
    delay(3000);
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_ncenB08_tr);
    u8g2.drawStr(0,15,"Sajac Transmitter");
    u8g2.drawStr(0,30,"Standby mode");
    u8g2.sendBuffer();
  }
}
