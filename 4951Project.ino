/*
 * --------------------------------------------------------------------------------------------------------------------
 * Example sketch/program showing how to read data from a PICC to serial.
 * --------------------------------------------------------------------------------------------------------------------
 * This is a MFRC522 library example; for further details and other examples see: https://github.com/miguelbalboa/rfid
 * 
 * Example sketch/program showing how to read data from a PICC (that is: a RFID Tag or Card) using a MFRC522 based RFID
 * Reader on the Arduino SPI interface.
 * 
 * When the Arduino and the MFRC522 module are connected (see the pin layout below), load this sketch into Arduino IDE
 * then verify/compile and upload it. To see the output: use Tools, Serial Monitor of the IDE (hit Ctrl+Shft+M). When
 * you present a PICC (that is: a RFID Tag or Card) at reading distance of the MFRC522 Reader/PCD, the serial output
 * will show the ID/UID, type and any data blocks it can read. Note: you may see "Timeout in communication" messages
 * when removing the PICC from reading distance too early.
 * 
 * If your reader supports it, this sketch/program will read all the PICCs presented (that is: multiple tag reading).
 * So if you stack two or more PICCs on top of each other and present them to the reader, it will first output all
 * details of the first and then the next PICC. Note that this may take some time as all data blocks are dumped, so
 * keep the PICCs at reading distance until complete.
 * 
 * @license Released into the public domain.
 * 
 * Typical pin layout used:
 * -----------------------------------------------------------------------------------------
 *             MFRC522      Arduino       Arduino   Arduino    Arduino          Arduino
 *             Reader/PCD   Uno/101       Mega      Nano v3    Leonardo/Micro   Pro Micro
 * Signal      Pin          Pin           Pin       Pin        Pin              Pin
 * -----------------------------------------------------------------------------------------
 * RST/Reset   RST          9             5         D9         RESET/ICSP-5     RST
 * SPI SS      SDA(SS)      10            53        D10        10               10
 * SPI MOSI    MOSI         11 / ICSP-4   51        D11        ICSP-4           16
 * SPI MISO    MISO         12 / ICSP-1   50        D12        ICSP-1           14
 * SPI SCK     SCK          13 / ICSP-3   52        D13        ICSP-3           15
 */

#include <SPI.h>
#include <MFRC522.h>
#include <stdlib.h>
#include <time.h>
#include <SoftwareSerial.h>
#include <TimeLib.h>

#define SS_PIN 10
#define RST_PIN 9
#define PIN_TRIG 2
#define PIN_ECHO 3
#define SSID "Small_iPhone"      //wifi名
#define PASS "wzy1234567890" //wifi密码
#define IP "api.heclouds.com" // 连接thingspeak.com服务器
const char OneNetServer[] = "api.heclouds.com";
const char APIKEY[] = "xHFf12P5QZeYa6wW3r4yeFhoFog=";    //API KEY
int32_t DeviceId = 704934617; 
const char card[]="Card";
const char distance[]="Distance";
const char time1[] = "Time";
const int tcpPort = 80;                        
float cm;
float temp;
double duration;
time_t start, finish;
 
MFRC522 rfid(SS_PIN, RST_PIN); // Instance of the class
SoftwareSerial monitor(6, 7); // 定义软串口RX, TX
MFRC522::MIFARE_Key key; 


byte nuidPICC[4]; // Init array that will store new UID 

void setup() { 
  Serial.begin(115200); //Set up the initial baud 9600
  monitor.begin(115200);
  
  monitor.println("AT");//指令测试
  delay(1000);
  if (monitor.find("OK"))  //接收指令正常则返回OK
  {
    Serial.println("Wifi module connection is normal");
    connectWiFi();
  }
  else {
    Serial.println("Wifi module connection failed");
  }
  pinMode(PIN_TRIG, OUTPUT);  
  pinMode(PIN_ECHO, INPUT);  
  SPI.begin(); // Init SPI bus
  Serial.println(F("Start--------------------"));
  rfid.PCD_Init(); // Init MFRC522 

  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }

  Serial.println(F("This code scan the MIFARE Classsic NUID."));
  Serial.print(F("Using the following key:"));
  printHex(key.keyByte, MFRC522::MF_KEY_SIZE);
}
 
void loop() {
// Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
  if ( ! rfid.PICC_IsNewCardPresent())
    return;

  // Verify if the NUID has been readed
  if ( ! rfid.PICC_ReadCardSerial())
    return;

  Serial.print(F("PICC type: "));
  MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
  Serial.println(rfid.PICC_GetTypeName(piccType));

  // Check is the PICC of Classic MIFARE type
  if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI &&  
    piccType != MFRC522::PICC_TYPE_MIFARE_1K &&
    piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
    Serial.println(F("Your tag is not of type MIFARE Classic."));
    return;
  }

  if (rfid.uid.uidByte[0] != nuidPICC[0] || 
    rfid.uid.uidByte[1] != nuidPICC[1] || 
    rfid.uid.uidByte[2] != nuidPICC[2] || 
    rfid.uid.uidByte[3] != nuidPICC[3] ) {
    Serial.println(F("A new card has been detected."));

    // Store NUID into nuidPICC array
    for (byte i = 0; i < 4; i++) {
      nuidPICC[i] = rfid.uid.uidByte[i];
    }
   
    Serial.println(F("The NUID tag is:"));
    Serial.print(F("In hex: "));
    printHex(rfid.uid.uidByte, rfid.uid.size);
    Serial.println();
  }
  else Serial.println(F("Card read previously."));
  

  start = now();
    
  cm=5;
  while(cm>4){
  Serial.println("????????");
  digitalWrite(PIN_TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(PIN_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_TRIG, LOW);
    
  temp = float(pulseIn(PIN_ECHO, HIGH));
  cm = (temp * 17 )/1000;
  Serial.print("Echo = ");  
  Serial.print(temp);
  Serial.print(",  Distance = ");  
  Serial.print(cm);
  Serial.println("cm");  
  delay(300); 
  char buffer[10];
  finish= now();
  duration = (double)(finish- start);
  String Time = dtostrf(duration,4,1,buffer);
  String Temp = dtostrf(temp, 4, 1, buffer);
  String Dist = dtostrf(cm, 4, 1, buffer);
  updateTemp(Temp,Dist,Time);
  delay(3000);
  }    

  rfid.PICC_HaltA();

  // Stop encryption on PCD
  rfid.PCD_StopCrypto1();




 
}
/**
 * Helper routine to dump a byte array as hex values to Serial. 
 */


boolean connectWiFi()
{
  //Serial.println("AT+CIPMUX=0");
  monitor.println("AT+CWMODE=1");
  monitor.println("AT+RST");
  delay(2000);
  String cmd = "AT+CWJAP=\"";
  cmd += SSID;
  cmd += "\",\"";
  cmd += PASS;
  cmd += "\"";
  monitor.println(cmd);
  delay(1000); 
   if(monitor.find("OK")) 
  {
    Serial.println("Wifi connection successful");
    return true;
  } else
  {
    Serial.println("Wifi connection failed");
    return false;
  }  
}

void printHex(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? "0" : "");
    Serial.print(buffer[i], HEX);
  } 
  }

String postData(int dId, String Card, String Distance, String Time) 
{
    // 创建发送请求的URL -- We now create a URI for the request
    String url = "/devices/704934617";//此处修改为你的设备id
    //url += String(dId);
    url += "/datapoints?type=3";           
    String data = "{\"" + String(card) + "\":" + Card + ",\"" +
                  String(distance) + "\":" + Distance + ",\"" +
                  String(time1) + "\":" + Time+"}";
    // 创建发送指令 -- We now combine the request to the server
    String post_data = "POST " + url + " HTTP/1.1\r\n" +
                       "api-key:" + APIKEY + "\r\n" +
                       "Host:" + OneNetServer + "\r\n" +
                       "Content-Length: " + String(data.length()) + "\r\n" +                     //发送数据长度
                       "Connection: close\r\n\r\n" +
                       data;
  
    // 发送指令 -- This will send the request to server
    return post_data;
}

void updateTemp(String card, String distance, String Time)
{
  String cmd = "AT+CIPSTART=\"TCP\",\"";
  cmd += IP;
  cmd += "\",80";
  //sendDebug(cmd);                        //发送指令，链接服务器
  monitor.println(cmd);
  delay(2000);
  if (monitor.find("Error"))
  {
    Serial.print("Connection to server failed");
    return;
  }
  //cmd = GET + "&field1=" + temph + "&field2=" + tempt + "\r\n";       //记录T和H的值
  cmd = postData(DeviceId, card, distance, Time);
  monitor.print("AT+CIPSEND=");
  monitor.println(cmd.length());
  if (monitor.find(">"))
  {
    Serial.print(">");
    monitor.print(cmd);
    Serial.print(cmd);
  }
  else
  {
    Serial.println("Data transmission failure");
  }
  if (monitor.find("OK"))
  {
    Serial.println("RECEIVED: OK");
    monitor.println("AT+CIPCLOSE");
  }
  else
  {
    Serial.println("Data transmission failure");
  }
}
