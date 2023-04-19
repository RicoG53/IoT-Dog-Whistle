#include <ETH.h>
#include <WiFi.h>
#include "secrets.h"
#include <M5StickCPlus.h>

const char* resource = "/trigger/classButtonA/with/key/gHMpHIGf7el9ssFbKM8hYPYazunbiT9BZGvwnK6ZMF_";
const char* server = "maker.ifttt.com";
const int button = G37;
int last_value = 0;
int cur_value = 0;

//speaker code
const int servo_pin = 26;
int freq = 80;
int ledChannel = 0;
int resolution = 10;
extern const unsigned char m5stack_startup_music[];

void setup() {
  Serial.begin(115200);
  M5.begin();
  delay(1000);
  
// screen display 
  M5.Lcd.fillScreen(BLUE);
  M5.Lcd.setTextColor(GREEN);
  M5.Lcd.setRotation(3);
  M5.Lcd.setTextSize(1);
  M5.Lcd.print("Dog Whistle");

  WiFi.begin(SECRET_SSID, SECRET_PASS);
  Serial.print("Connecting to  ");
  Serial.println(SECRET_SSID);
  while (WiFi.status()!= WL_CONNECTED) {
     Serial.print(".");
     delay(500);
  }
   Serial.println();
   Serial.print("Connecting to ");
   Serial.println(SECRET_SSID);

//init speaker setup
  M5.begin();
  M5.Lcd.setRotation(0);
  M5.Lcd.setCursor(25, 80, 4);
  ledcSetup(ledChannel, freq, resolution);
  ledcAttachPin(servo_pin, ledChannel);
  ledcWrite(ledChannel, 256);//0°
}

//further speaker coding
void playMusic(const uint8_t* music_data, uint16_t sample_rate) {
  uint32_t length = strlen((char*)music_data);
  uint16_t delay_interval = ((uint32_t)1000000 / sample_rate);
    for(int i = 0; i < length; i++) {
      ledcWriteTone(ledChannel, music_data[i]*50);
      delayMicroseconds(delay_interval);
    } 
    
}

// ----------------------------------------------------------------------------------------------------------------

void loop() {
  M5.update();
  if(M5.BtnA.wasPressed()){
     makeIFTTTRequest();
     M5.Lcd.println("Whistle Sounded, email notification sent...");    
   }

//speaker code
  cur_value = digitalRead(button);// read the value of BUTTON

  M5.Lcd.setCursor(40,25, 2); M5.Lcd.print("Dog Whistle");

  if(cur_value != last_value){
    M5.Lcd.fillRect(135,25,100,25,BLUE);
    M5.Lcd.fillRect(135,45,100,25,BLUE);
    if(cur_value==0){
      M5.Lcd.setCursor(135,25, 2); M5.Lcd.print("pressed");// display the status
      M5.Lcd.setCursor(135,45, 2); M5.Lcd.print("0");
      
      // change background 
      M5.Lcd.fillScreen(RED);
      
      Serial.println("Button Status: pressed");
      Serial.println("       value:  0");

      // make button activate speaker beep
      ledcWriteTone(ledChannel, 1250);
      delay(1000);
      ledcWriteTone(ledChannel, 0);
      delay(1000);
    }
    else{
      M5.Lcd.setCursor(135,25, 2); M5.Lcd.print("released");// display the status
      M5.Lcd.setCursor(135,45, 2); M5.Lcd.print("1");
      Serial.println("Button Status: released");
      Serial.println("       value:  1");
      M5.Lcd.fillScreen(BLUE);
    }
    last_value = cur_value;
  }
  
}

// ---------------------------------------------------------------------------------------------------------------------

void makeIFTTTRequest() {
  Serial.print("Connecting to ");
  Serial.print(server);
  WiFiClient client;
  int retries = 5;
  while(!!!client.connect(server, 80) && (retries-- > 0)) {
    Serial.print(".");
  }
    Serial.println();
    if(!!!client.connected()) {
      Serial.print("Failed to connect, going back to sleep.");
    }
    Serial.print("Request Resource: ");
    Serial.println(resource);
    client.print(String("GET ") + resource +
      " HTTP/1.1\r\n" + 
      "Host: " + server + "\r\n" + 
      "Connection: close\r\n\r\n");

    //time is 5 sec worth of 100 mSec  delay
    int timeout = 5 * 10;
    while(!!!client.available() && (timeout-- > 0)) {
        delay(100);
    }

    if(!!!client.available()){
        Serial.print("No response, going back to sleep.");   
    }

    while(client.available()){
        Serial.write(client.read());  
    }

    Serial.println("closing connection");
    client.stop();
   
}
