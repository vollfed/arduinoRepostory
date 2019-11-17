/***************************************************
  Adafruit MQTT Library ESP8266 Example

  Must use ESP8266 Arduino from:
    https://github.com/esp8266/Arduino

  Works great with Adafruit's Huzzah ESP board & Feather
  ----> https://www.adafruit.com/product/2471
  ----> https://www.adafruit.com/products/2821

  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Tony DiCola for Adafruit Industries.
  MIT license, all text above must be included in any redistribution
 ****************************************************/
#include <ESP8266WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

/************************* WiFi Access Point *********************************/

#define WLAN_SSID       "XXXX"
#define WLAN_PASS       "XXXX"

/************************* Adafruit.io Setup *********************************/

#define AIO_SERVER      "192.168.3.11"
#define AIO_SERVERPORT  1883                   // use 8883 for SSL
#define AIO_USERNAME    ""
#define AIO_KEY         ""

/************ Global State (you don't need to change this!) ******************/

// Create an ESP8266 WiFiClient class to connect to the MQTT server.
WiFiClient client;
// or... use WiFiFlientSecure for SSL
//WiFiClientSecure client;

// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

/****************************** Feeds ***************************************/

// Setup a feed called 'sensValue' for publishing.
// Notice MQTT paths for AIO follow the form: <username>/feeds/<feedname>
Adafruit_MQTT_Publish sensValue = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/f/outTopic");

// Setup a feed called 'ledBrightness' for subscribing to changes.
Adafruit_MQTT_Subscribe ledBrightness = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/f/inTopic");

/*************************** Sketch Code ************************************/

// Bug workaround for Arduino 1.6.6, it seems to need a function declaration
// for some reason (only affects ESP8266, likely an arduino-builder bug).
void MQTT_connect();

uint8_t ledPin = 13;
uint16_t sensAdcValue = 0;
uint16_t ledBrightValue = 0;

void setup() {
  Serial.begin(9600);
  delay(10);

  Serial.println(F("Adafruit MQTT demo"));
  pinMode(ledPin, OUTPUT); 
  
  // Connect to WiFi access point.
  Serial.println(); Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WLAN_SSID);

  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    digitalWrite(ledPin, LOW);
    delay(500);
    digitalWrite(ledPin, HIGH);
    Serial.print(".");
  }
  Serial.println();

  Serial.println("WiFi connected");
  Serial.println("IP address: "); Serial.println(WiFi.localIP());

  // Setup MQTT subscription for ledBrightness feed.
  mqtt.subscribe(&ledBrightness);
}

void loop() {
  // Ensure the connection to the MQTT server is alive (this will make the first
  // connection and automatically reconnect when disconnected).  See the MQTT_connect
  // function definition further below.
  MQTT_connect();

  // this is our 'wait for incoming subscription packets' busy subloop
  // try to spend your time here

  Adafruit_MQTT_Subscribe *subscription;
  while ((subscription = mqtt.readSubscription(200))) {
    if (subscription == &ledBrightness) {
      Serial.print(F("Got LED Brightness : "));
      ledBrightValue = atoi((char *)ledBrightness.lastread);
      Serial.println(ledBrightValue);

      if(ledBrightValue == 1){
         Serial.println("on");
         digitalWrite(ledPin, HIGH);
      } else {
        Serial.println("off");
        digitalWrite(ledPin, LOW);
      } 
    }
  }

  // Now we can publish stuff!
  uint16_t AdcValue = analogRead(A0);
  // if(sensAdcValue != AdcValue){
    sensAdcValue = AdcValue;
    Serial.print(F("Sending sensor val: "));
    Serial.println(sensAdcValue);
    //Serial.print("...");
    if (! sensValue.publish(sensAdcValue)) {
      //Serial.println(F("Failed"));
    } else {
      //Serial.println(F("OK!"));
    }
    delay(50);
  //}
  // ping the server to keep the mqtt connection alive
  // NOT required if you are publishing once every KEEPALIVE seconds
  /*
  if(! mqtt.ping()) {
    mqtt.disconnect();
  }
  */
  //delay(100);
}

// Function to connect and reconnect as necessary to the MQTT server.
// Should be called in the loop function and it will take care if connecting.
void MQTT_connect() {
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }

  Serial.print("Connecting to MQTT... ");

  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
       Serial.println(mqtt.connectErrorString(ret));
       Serial.println("Retrying MQTT connection in 5 seconds...");
       mqtt.disconnect();
       digitalWrite(ledPin, HIGH);
       delay(5000);  // wait 5 seconds
       digitalWrite(ledPin, LOW);
       retries--;
       if (retries == 0) {
         // basically die and wait for WDT to reset me
         while (1);
       }
  }
  Serial.println("MQTT Connected!");
}
