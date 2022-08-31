// Enter your WiFi ssid and password
const char* ssid     = "Room 4004A";   //your network SSID
const char* password = "room4004";   //your network password

String myScript = "/macros/s/AKfycbxbG_Ru_akMWU9AXsl577K9HChSNbKGliTSGt3akVQwASEwtgZ-/exec";    //Create your Google Apps Script and replace the "myScript" path.
String myLineNotifyToken = "myToken=**********";    //Line Notify Token. You can set the value of xxxxxxxxxx empty if you don't want to send picture to Linenotify.
String myFoldername = "&myFoldername=Captured Images";
String myFilename = "&myFilename=Capture00.jpg";
String myImage = "&myFile=";


unsigned long myChannelNumber = 1837375;
const char * myWriteAPIKey = "Q8LAAYYD9GL30WFU";

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include "Base64.h"
#include "DHT.h"
#include "secrets.h"
#include "ThingSpeak.h"
#include "esp_camera.h"

WiFiClient  client;

// WARNING!!! Make sure that you have either selected ESP32 Wrover Module,
//            or another board which has PSRAM enabled

//CAMERA_MODEL_AI_THINKER
uint8_t DHTPin = 14; 
#define DHTTYPE    DHT11     // DHT 11
DHT dht(DHTPin, DHTTYPE);                
float Temperature;
float Humidity;

#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22
#define soilPower 2
#define soilPin 13
int motor = 12;
int flash = 4;

void setup()
{
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();
  delay(10);
  pinMode(soilPower, OUTPUT);
  pinMode(flash, OUTPUT);
  digitalWrite(soilPower, LOW);
  pinMode(motor,OUTPUT);
  digitalWrite(motor,HIGH);
  pinMode(DHTPin, INPUT);
  dht.begin();
  WiFi.mode(WIFI_STA);
  ThingSpeak.begin(client); 
  Serial.println("");
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);  
  
  long int StartTime=millis();
  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(500);
    if ((StartTime+10000) < millis()) break;
  } 

  Serial.println("");
  Serial.println("STAIP address: ");
  Serial.println(WiFi.localIP());
    
  Serial.println("");

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Reset");
    
    ledcAttachPin(4, 3);
    ledcSetup(3, 5000, 8);
    ledcWrite(3,10);
    delay(200);
    ledcWrite(3,0);
    delay(200);    
    ledcDetachPin(3);
        
    delay(1000);
    ESP.restart();
  }
  else {
    ledcAttachPin(4, 3);
    ledcSetup(3, 5000, 8);
    for (int i=0;i<5;i++) {
      ledcWrite(3,10);
      delay(200);
      ledcWrite(3,0);
      delay(200);    
    }
    ledcDetachPin(3);      
  }

  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  //init with high specs to pre-allocate larger buffers
  if(psramFound()){
    config.frame_size = FRAMESIZE_UXGA;
    config.jpeg_quality = 10;  //0-63 lower number means higher quality
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;  //0-63 lower number means higher quality
    config.fb_count = 1;
  }
  
  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    delay(1000);
    ESP.restart();
  }

  //drop down frame size for higher initial frame rate
  sensor_t * s = esp_camera_sensor_get();
  s->set_framesize(s, FRAMESIZE_VGA);  // UXGA|SXGA|XGA|SVGA|VGA|CIF|QVGA|HQVGA|QQVGA
}

void loop()
{ 
  digitalWrite(motor,HIGH);
  int external=externalReadMode();
  if(external==1)
  manualmode();
  else
  automode();
 

}

void automode()
{  
  Serial.println("Auto Mode ON \n");
   dht11Read(0);
  int external;
  for(int i=0;i<7;i++)
  
  {
   external=externalReadMode();
   if(external==1)
  manualmode();
  else
    {
    delay(2000);
    }
    }
  Serial.println("Get ready for the Image!\n");
  digitalWrite(flash,HIGH);
  SendCapturedImage();
  digitalWrite(flash,LOW);
   Serial.println("Captured!\n");
    for(int i=0;i<15;i++)
  {
   external=externalReadMode();
   if(external==1)
  manualmode();
  else
    {
    delay(2000);
    }
    }
  ESP.restart();
  }

int flag=5;
int manualmode()
{ 
  Serial.println("manual Mode ON \n");
  int pump=externalReadPump();
  int modeselect=externalReadMode();
  if(modeselect==1)
  {
  if(pump==1)
  {
    if(flag==0)
    {
      }
    else
    {
    flag=0;
    digitalWrite(motor,LOW);
    delay(15000);
    ThingSpeak.writeField(1837375,3,0,"Q8LAAYYD9GL30WFU");
    Serial.print("Pump Status is updated with ON\n");
    delay(2000);
    }
    }
  else
  { 
    if(flag==1)
    {
      delay(2000);
      }
    else
    { 
    digitalWrite(motor,HIGH);
    flag=1;
    delay(15000); 
    ThingSpeak.writeField(1837375,3,1,"Q8LAAYYD9GL30WFU");
    Serial.print("Pump Status is updated with OFF\n");
    delay(2000);
    }
  }
  manualmode();
  }
  else
  {
  digitalWrite(motor,HIGH);
  SendCapturedImage();
  delay(20000);
  ESP.restart();
  return 0;
  }
  }


int externalReadMode()
{
  long i=ThingSpeak.readLongField( 1837375,5,"YRI73TU4PDT07UVJ");
  i=int(i);
  int statusCode = ThingSpeak.getLastReadStatus();
   if (statusCode == 200)
  {
   
    return i;
  }
  else
  {
    Serial.println("Unable to read channel / No internet connection");
  }
  delay(100);

  }
int externalReadPump()
{
  long i=ThingSpeak.readLongField( 1837375,6,"YRI73TU4PDT07UVJ");
  i=int(i);
  int statusCode = ThingSpeak.getLastReadStatus();
   if (statusCode == 200)
  {
    Serial.print("Pump Status is Read from server\n");
    return i;
  }
  else
  {
    Serial.println("Unable to read channel / No internet connection");
  }
  delay(100);

  }
int startMotor(int val)
{
   if(val)
  {
   if(motor==LOW)
    {
      }
    else
    {
    digitalWrite(motor,LOW);
    Serial.println("Motor is watering");
    }
   val=soilRead();
   delay(3000);
   //val=externalRead();
   int external=externalReadMode();
   if(external==1)
   {
    manualmode();
   }
   startMotor(val);
   //startMotor(0);
    
  }
  else
   {
    digitalWrite(motor,HIGH);
    Serial.println("Motor is turned off");
    //ThingSpeak.writeField(1837375,3,1,"Q8LAAYYD9GL30WFU");
    delay(1000);
  }
  }
  
int soilRead() {
  digitalWrite(soilPower, HIGH);  // Turn the sensor ON
  delay(30);              // Allow power to settle
  int val = digitalRead(soilPin); // Read the analog value form sensor
  digitalWrite(soilPower, LOW);   // Turn the sensor OFF
  if (val) 
  {
    Serial.println("Status: Soil is too dry - time to water!");
    //ThingSpeak.writeField(1837375,3,0,"Q8LAAYYD9GL30WFU");
    //Serial.println("Moisture is updated with 0 ");
    delay(200);
  } 
  else 
  {
    Serial.println("Status: Soil moisture is perfect");
    //ThingSpeak.setField(3,1);
    //ThingSpeak.writeFields(1837375,"Q8LAAYYD9GL30WFU");
    //Serial.println("Moisture is updated with 1 ");
    delay(200);
  }
  return val;            
}

int i=0;
int dht11Read(int i)
{
delay(500);
Serial.println(i);
float  t=readDHTTemperature();
float  h=readDHTHumidity();
//int val=soilRead();
int k=int(t);
  if(int(k)!=0)
  {
   int val=soilRead();
   int valn;
   if(val==0)
   valn=1;
   else
   valn=0;
   dataToThingSpeak(h,t,valn);
   if(val)
   {
   startMotor(val);
   dataToThingSpeak(h,t,1);
   }
   delay(500);
   return 0;
   }
  else
   {   
  delay(500);
  i++;
  if(i==4)
  return 0;
  else
  dht11Read(i);
   }
}
void dataToThingSpeak(float humidity,float temp,int moisture)
  {
     ThingSpeak.setField(2,temp);
     //ThingSpeak.setField(3,moisture);
     ThingSpeak.setField(4,humidity);  
     ThingSpeak.setField(3,moisture);
     ThingSpeak.writeFields(1837375,"Q8LAAYYD9GL30WFU");
     Serial.println("Channel is updated with: ");
     Serial.println(humidity);
     Serial.println("and");
     Serial.println(temp);
     Serial.println("and");
     Serial.println(moisture);
     delay(1000);
  }
  
float readDHTTemperature() {
  float t = dht.readTemperature();
  if (isnan(t)) {    
    Serial.println("Failed to read from DHT sensor!");
    return 0;
  }
  else {
    Serial.println(t);
    return t;
  }
}

float readDHTHumidity() {
  float h = dht.readHumidity();
  if (isnan(h)) {
    Serial.println("Failed to read from DHT sensor!");
    return 0;
  }
  else {
    Serial.println(h);
    return h;
  }
}
String SendCapturedImage() {
  const char* myDomain = "script.google.com";
  String getAll="", getBody = "";
  
  camera_fb_t * fb = NULL;
  fb = esp_camera_fb_get();  
  if(!fb) {
    Serial.println("Camera capture failed");
    delay(1000);
    ESP.restart();
    return "Camera capture failed";
  }  
  
  Serial.println("Connect to " + String(myDomain));
  WiFiClientSecure client_tcp;
  client_tcp.setInsecure();   //run version 1.0.5 or above
  
  if (client_tcp.connect(myDomain, 443)) {
    Serial.println("Connection successful");
    
    char *input = (char *)fb->buf;
    char output[base64_enc_len(3)];
    String imageFile = "data:image/jpeg;base64,";
    for (int i=0;i<fb->len;i++) {
      base64_encode(output, (input++), 3);
      if (i%3==0) imageFile += urlencode(String(output));
    }
    String Data = myLineNotifyToken+myFoldername+myFilename+myImage;
    
    client_tcp.println("POST " + myScript + " HTTP/1.1");
    client_tcp.println("Host: " + String(myDomain));
    client_tcp.println("Content-Length: " + String(Data.length()+imageFile.length()));
    client_tcp.println("Content-Type: application/x-www-form-urlencoded");
    client_tcp.println("Connection: keep-alive");
    client_tcp.println();
    
    client_tcp.print(Data);
    int Index;
    for (Index = 0; Index < imageFile.length(); Index = Index+1000) {
      client_tcp.print(imageFile.substring(Index, Index+1000));
    }
    esp_camera_fb_return(fb);
    
    int waitTime = 10000;   // timeout 10 seconds
    long startTime = millis();
    boolean state = false;
    
    while ((startTime + waitTime) > millis())
    {
      Serial.print(".");
      delay(100);      
      while (client_tcp.available()) 
      {
          char c = client_tcp.read();
          if (state==true) getBody += String(c);        
          if (c == '\n') 
          {
            if (getAll.length()==0) state=true; 
            getAll = "";
          } 
          else if (c != '\r')
            getAll += String(c);
          startTime = millis();
       }
       if (getBody.length()>0) break;
    }
    client_tcp.stop();
    Serial.println(getBody);
  }
  else {
    getBody="Connected to " + String(myDomain) + " failed.";
    Serial.println("Connected to " + String(myDomain) + " failed.");
  }
  
  return getBody;
}


String urlencode(String str)
{
    String encodedString="";
    char c;
    char code0;
    char code1;
    char code2;
    for (int i =0; i < str.length(); i++){
      c=str.charAt(i);
      if (c == ' '){
        encodedString+= '+';
      } else if (isalnum(c)){
        encodedString+=c;
      } else{
        code1=(c & 0xf)+'0';
        if ((c & 0xf) >9){
            code1=(c & 0xf) - 10 + 'A';
        }
        c=(c>>4)&0xf;
        code0=c+'0';
        if (c > 9){
            code0=c - 10 + 'A';
        }
        code2='\0';
        encodedString+='%';
        encodedString+=code0;
        encodedString+=code1;
        //encodedString+=code2;
      }
      yield();
    }
    return encodedString;
}
