	#include <Adafruit_GFX.h>
	#include <Adafruit_SSD1306.h>
	#include <Arduino.h>
	#include <esp_now.h>
	#include <WiFi.h>
	#include <Wire.h>
	#include <esp_wifi.h>

	uint8_t broadcastAddress[] = {0xC8, 0xC9, 0xA3, 0xFC, 0xDC, 0x5C};
	bool channelFound = false;

	#define OLED_RESET 4
	Adafruit_SSD1306 display(OLED_RESET);

	typedef struct struct_message {
	    int id;
	    int x;
	    int y;
	    int z;
	} struct_message;

	struct_message myData;


	uint8_t channel = 1;

	void tryNextChannel() {
	Serial.println("Changing channel from " + String(channel) + " to " + String(channel+1));
	channel = channel % 13 + 1;
	esp_wifi_set_promiscuous(true);
	esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE);
	esp_wifi_set_promiscuous(false);
	}

	void onDataSent(const uint8_t* mac_addr, esp_now_send_status_t status) {
	if (!channelFound && status != ESP_NOW_SEND_SUCCESS) {
		Serial.println("Delivery Fail because channel" + String(channel) + " does not match receiver channel.");
		tryNextChannel(); // If message was not delivered, it tries on another wifi channel.
	}

	else {
		Serial.println("Delivery Successful ! Using channel : " + String(channel));
		channelFound = true;
	}
	}
	const int analogIn = A0;
	int rawValue = 0;
	double voltage = 0;
	double rawSum = 0;
	int limit = 2000;
	int dataToSend = 0;

	void displayValues(){

	  rawSum = analogRead(analogIn);
	  analogReadResolution(12);

	  // ornekleme 
	  for (int i = 0; i < 500; i++){         
	      rawSum += analogRead(analogIn);
	  }

	  rawValue = rawSum/500;
	  rawValue = rawValue; 
	  voltage = (rawValue / 4096.0) * 3300;
	  Serial.print("adc value = " );    
	  Serial.print(rawValue);
	  Serial.print("\n");
	  Serial.print("mvolts = ");   
	  Serial.print(voltage,0);
	  Serial.print("\n");
	  delay(300);

	  display.clearDisplay();    
	  display.setTextColor(WHITE); 
	  display.setTextSize(1);      
	  display.setCursor(0,0);         

	  if (rawValue<limit){          
	    digitalWrite(LED_BUILTIN, LOW);
	    digitalWrite(18, LOW);
	    display.print("NORMAL");
	    display.setCursor(0,10); 
	    display.print("raw value:"); 
	    display.print(rawValue);
	    display.print("\n");
	  }
	  else {
	    digitalWrite(LED_BUILTIN, HIGH);
	    digitalWrite(18, HIGH);
	    display.print("LEAK");
	    display.setCursor(0,10); 
	    display.print("raw value:"); 
	    display.print(rawValue);
	    display.print("\n");
	  }
	}

	void setup() {
	  pinMode (LED_BUILTIN, OUTPUT);  
	  pinMode (18, OUTPUT);
	  Serial.begin(115200);  
	  Wire.begin();
	  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);   // oled i2c adress 0x3C

	  WiFi.mode(WIFI_STA);      

	  if (esp_now_init() != ESP_OK) {
	    Serial.println("Error initializing ESP-NOW");
	    return;
	  }


	//https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/network/esp_now.html#send-esp-now-data
	esp_now_register_send_cb(onDataSent);
	  
    esp_now_peer_info_t peerInfo;
	memset(&peerInfo, 0, sizeof(peerInfo));
	memcpy(peerInfo.peer_addr, broadcastAddress, 6);
	peerInfo.channel = 0;  
	peerInfo.encrypt = false;
	  
	if (esp_now_add_peer(&peerInfo) == ESP_OK) {
		Serial.println("Pair success");
	}
	else
	{
		Serial.println("Pair failed");
	}

	  delay(300);              
	}

	void loop() {

	  myData.id = 1;
	  dataToSend = rawValue;
	  myData.x = dataToSend;
	  myData.y = random(0,50);
	  myData.z = random(0,50);
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
