#include <Arduino.h>
#include "TFT_eSPI.h"
#include <SoftwareSerial.h>

SoftwareSerial e5(0, 1);

int data1;
int data2;
int data3;
int data4;

static char recv_buf[512];
static bool is_exist = false;

TFT_eSPI tft;
TFT_eSprite spr = TFT_eSprite(&tft);  //sprite

  
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
 
static int recv_prase(void)
{
  char ch;
  int index = 0;
  memset(recv_buf, 0, sizeof(recv_buf));
  while (e5.available() > 0){
    ch = e5.read();
    recv_buf[index++] = ch;
    Serial.print((char)ch);
    delay(2);
  }

  if (index){
    char *p_start = NULL;
    char data[32] = {0,};
    int rssi = 0;
    int snr = 0;
  
    p_start = strstr(recv_buf, "+TEST: RX \"5345454544");
    if (p_start)
    {
      p_start = strstr(recv_buf, "5345454544");
      if (p_start && (1 == sscanf(p_start, "5345454544%s,", data)))
      {
        data[16] = 0;
        char *endptr;
        char *endptr1;
        char *endptr2;
        char *endptr3;
        char dataarray1[5] = {data[0], data[1],data[2], data[3]};
        char dataarray2[5] = {data[4], data[5], data[6], data[7]};
        char dataarray3[5] = {data[8], data[9], data[10], data[11]};
        char dataarray4[5] = {data[12], data[13],data[14], data[15]};
        
        data1 = strtol(dataarray1, &endptr, 16);
        data2 = strtol(dataarray2, &endptr1, 16);
        data3 = strtol(dataarray3, &endptr, 16);
        data4 = strtol(dataarray4, &endptr1, 16);
        
        spr.createSprite(100, 30);
        spr.fillSprite(TFT_WHITE);
        spr.setFreeFont(&FreeSansBoldOblique12pt7b);
        spr.setTextColor(TFT_BLACK);
        spr.drawNumber(data1, 0, 0, 1);
        spr.pushSprite(160, 65);
        spr.deleteSprite();
        
        spr.createSprite(100, 30);
        spr.fillSprite(TFT_WHITE);
        spr.drawNumber(data2, 0, 0, 1);
        spr.pushSprite(160, 120);
        spr.deleteSprite();

        spr.createSprite(100, 30);
//        spr.fillSprite(TFT_WHITE);
        if(data3 == 1){
          spr.fillSprite(TFT_GREEN);
          spr.drawString("Save", 0 , 0 , 1);
        }else if (data3 == 2){
          spr.fillSprite(TFT_ORANGE);
          spr.drawString("Bad", 0 , 0 , 1);
        }else if (data3 == 3){
          spr.fillSprite(TFT_RED);
          spr.drawString("S O S", 0 , 0 , 1);
          spr.pushSprite(160, 175);
          spr.deleteSprite();

          for (int i = 0; i<4; i++){
            analogWrite(WIO_BUZZER, 128);
            delay(1000);
            analogWrite(WIO_BUZZER, 0);
            delay(1000);
          }
        }
        
        spr.pushSprite(160, 175);
        spr.deleteSprite();
        
        Serial.print("data received displaying on the wio terminal");
        Serial.print("\r\n");
      }
  
        p_start = strstr(recv_buf, "RSSI:");
        if (p_start && (1 == sscanf(p_start, "RSSI:%d,", &rssi)))
        {
            String newrssi = String(rssi);
      
            Serial.print(rssi);
            Serial.print("\r\n");
  
        }
        p_start = strstr(recv_buf, "SNR:");
        if (p_start && (1 == sscanf(p_start, "SNR:%d", &snr)))
        {
            Serial.print(snr);
            Serial.print("\r\n");
  
            
        }
        return 1;
        }
    }
    return 0;
}
 
static int node_recv(uint32_t timeout_ms)
{
    at_send_check_response("+TEST: RXLRPKT", 1000, "AT+TEST=RXLRPKT\r\n");
    int startMillis = millis();
    do
    {
        if (recv_prase())
        {
            return 1;
        }
    } while (millis() - startMillis < timeout_ms);
    return 0;
}
 
void display1(){
  tft.fillScreen(TFT_WHITE);
  tft.fillRect(0,0,320,60,TFT_CYAN);
  tft.drawLine(0,60,320,60,TFT_BLACK);
  tft.setFreeFont(&FreeSansBoldOblique12pt7b);
  tft.setTextColor(0x39C7);
  
  tft.drawString("Checkpoint :", 7 , 65 , 1);
  tft.drawString("User ID       :", 7 , 120 , 1);
  tft.drawString("Status        :", 7 , 175 , 1);

  tft.setTextColor(TFT_BLACK);
  tft.setFreeFont(&FreeSansBoldOblique18pt7b);
  tft.drawString("SAJAC GUARDS", 15, 10 , 1);
}
 
void setup(void)
{
 
  tft.begin();
  tft.setRotation(3);
  Serial.begin(115200);

  pinMode(WIO_BUZZER, OUTPUT);
  // while (!Serial);

  e5.begin(9600);
  Serial.print("Receiver\r\n");

  display1();
  
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
 
void loop(void){
  if (is_exist){
    node_recv(2000);
  }
}
