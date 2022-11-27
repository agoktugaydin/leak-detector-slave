
// oled ve pio icin kutuphaneler dahil edildi
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Arduino.h>
//esp-now protokolu icin gerekli kutuphaneler
#include <esp_now.h>
#include <WiFi.h>
// i2c kutuphanesi icin gerekli
#include <Wire.h>

// alici cihazin mac adressi
uint8_t broadcastAddress[] = {0xC8, 0xC9, 0xA3, 0xFC, 0xDC, 0x5C};

// reset pini kullanilmiyor fakat i2c kutuphanesi icin gerekli
#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);

// gonderilecek veri icin gerekli struct yapisi
// alici cihazdaki struct yapisi ile eslesmeli
typedef struct struct_message {
    int id; // verici cihaza ait unique id
    double x; // adc value
    int y;
} struct_message;

struct_message myData;
esp_now_peer_info_t peerInfo;

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

// pin and constant value assignees
const int analogIn = A0;
double rawValue = 0;
double voltage = 0;
double rawSum = 0;
int limit = 1000;

void displayValues(){

  rawSum = analogRead(analogIn);
  analogReadResolution(12);

  // ornekleme 
  for (int i = 0; i < 500; i++){         
      rawSum += analogRead(analogIn);
  }

  rawValue = rawSum/500;
  voltage = (rawValue / 4096.0) * 3300; // sensoru besleyen kaynagin voltaji onemli
  Serial.print("adc value = " );        // adc value
  Serial.print(rawValue);
  Serial.print("\n");
  Serial.print("mvolts = ");    // shows the voltage measured
  Serial.print(voltage,0);
  Serial.print("\n");
  delay(300);

  display.clearDisplay();         // clear display
  display.setTextColor(WHITE);    //set color
  display.setTextSize(1);         //set font size
  display.setCursor(0,0);         //set cursor coordinates

  if (rawValue<limit){            // limitleme -DEGERLE ILGILENILECEK-----------------------------------------
    display.print("NORMAL");
    display.setCursor(0,10); 
    display.print("raw value:"); 
    display.print(rawValue);
    display.print("\n");
  }
  else {
    display.print("LEAK");
    display.setCursor(0,10); 
    display.print("raw value:"); 
    display.print(rawValue);
    display.print("\n");
  }
}

void setup() {
  Serial.begin(115200);  
  Wire.begin();
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);   // oled i2c adress 0x3C
  // cihaz wifi station olarak kuruldu
  WiFi.mode(WIFI_STA);      
  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  //esp-now kurulumu sonrasi kontrol -detayli incele-
  //https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/network/esp_now.html#send-esp-now-data
  esp_now_register_send_cb(OnDataSent);
  
  // alici kartla esleme yapildi
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
  Serial.println("Failed to add peer");
  return;
  }

  delay(300);               // sensorun stabil hale gelmesi icin delay
}

void loop() {
  

  // Set values to send
  myData.id = 1;
  myData.x = rawValue;
  myData.y = random(0,50);
  // mesaji gonder
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
   
  if (result == ESP_OK) {
    Serial.println("Sent with success");
  }
  else {
    Serial.println("Error sending the data");
  }

  displayValues();
  display.display();


  delay(300);
}
