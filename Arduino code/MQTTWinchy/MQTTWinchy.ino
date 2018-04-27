///////////////////
// BYTEK PHOENIX //
///////////////////

/*
  MQTTWinchy  
  Sends and receives data via MQTT, to be used with mqtt-monitoring app.
  This sketch is made for the bytek phoenix board and Winchy chassis.
  If you want to know more about this board, 
  check our Github at https://github.com/bytekengineering/phoenix
  
  This example code is in the public domain.
  More info at http://bytek.info/
*/

#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <Adafruit_NeoPixel.h>

// How many NeoPixels are attached to the Arduino?
#define NUMPIXELS      4
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, D9, NEO_GRB + NEO_KHZ800);

// Wifi configuration
const char* ssid = "your-ssid";
const char* password = "your-pass";


// mqtt configuration
const char* server = "your-server-ip";
const char* topic1 = "phoenix/winchy/dist";
const char* topic2 = "phoenix/winchy/temp";
const char* topic3 = "phoenix/winchy/luz";
const char* clientName = "com.winchy.phoenix";

// Pin assigment to sensors
#define TRIGGERPIN D10
#define ECHOPIN    D8
#define SENSOR A0  //  LM35

// Vars def
int duration, distance;
int cont;
int red, green, blue;
int value;
float temp;
String payload;
WiFiClient wifiClient;
PubSubClient client(wifiClient);

void setup() {
    client.setServer(server, 1883);
    client.setCallback(callback);
    pixels.begin();
    wifiConnect();
    cont=0;
     pinMode(TRIGGERPIN, OUTPUT); // Sets the trigPin as an Output
   pinMode(ECHOPIN, INPUT); // Sets the echoPin as an Input
    delay(1500);
}
void loop() {
    // Mide distancia
    // Clears the trigPin
    digitalWrite(TRIGGERPIN, LOW);  
    delayMicroseconds(3); 
    
    // Sets the trigPin on HIGH state for 12 micro seconds
    digitalWrite(TRIGGERPIN, HIGH);
    delayMicroseconds(12);  
    digitalWrite(TRIGGERPIN, LOW);
  
    // Reads the echoPin, returns the sound wave travel time in microseconds
    duration = pulseIn(ECHOPIN, HIGH);
    // Calculating the distance
    distance = (duration*0.034) / 2;   
  
    payload = (String) distance;

    // Envía la distancia
    if (client.connected()) {
       client.publish(topic1, (char*) payload.c_str());
    } else {
        mqttReConnect();
    }

    // Cada 20 segundos se va a actualizar la temperatura
    if (cont>50) {
      // Calcula temperatura
      value = analogRead(SENSOR);
      
      int val_step = 3300/1024;
      // Convert analog value to temperature       
      temp = ( value * val_step) / 10;
      payload = (String) temp;

      // Envía la temperatura
      if (client.connected()) {
          client.publish(topic2, (char*) payload.c_str());
      } else {
          mqttReConnect();
      }
      cont=0;
    }
    
    cont=cont+1;
    delay(500);
  client.loop();
}

void wifiConnect() {
    // Se conecta al WiFi
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
    }
     
    if (client.connect(clientName)) {
        client.subscribe(topic3);

    }
    else {
        abort();
    }

}


void mqttReConnect() {
    // Reconexiones
   while (!client.connected()) {
    // Attempt to connect
    if (client.connect(clientName)) {
      client.subscribe(topic3);
    } else {
      delay(5000);
    }
  }
}
void callback(char* topic, byte* payload, unsigned int length) {

  String data;

  // To use it with MQTT Dash Mobile App
  if (length>6) {
    for (int i=1; i < length; i++) {
        data += (char)payload[i];
     }  
  }
   else {
    for (int i=0; i < length; i++) {
        data += (char)payload[i];
      }  
   }
  long rgb = (long) strtol( &data[0], NULL, 16);

  convertRGB(rgb);


  for(int i=0;i<NUMPIXELS;i++) {      
    pixels.setPixelColor(i, red,green,blue);      
  }
   
  pixels.show(); // This sends the updated pixel color to the hardware.

}
  
void convertRGB(long number) {
    red = number >> 16;
    green = number >> 8 & 0xFF;
    blue = number & 0xFF;
}
