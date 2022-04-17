/******************************************************/
//       THIS IS A GENERATED FILE - DO NOT EDIT       //
/******************************************************/

#include "Particle.h"
#line 1 "d:/DEVELOPMENT/Radio_Eastern/particle_argon/doorbell/src/doorbell.ino"
#include <MQTT.h>
#include "Debounce/Debounce.h"

void setup();
void loop();
#line 4 "d:/DEVELOPMENT/Radio_Eastern/particle_argon/doorbell/src/doorbell.ino"
SYSTEM_THREAD(ENABLED);

Debounce doorbellDebounced = Debounce();

const String url = "192.168.0.26";
const String topic = "efm/sensors/doorbell";
const String deviceName = "MBlashkiArgon";
const String MQTTusername = "MBlashkiArgon";
const String MQTTpassword = "1234";
const pin_t Doorbell = D6;
const pin_t LED = D7;

// Declarations
void callback(char* topic, byte* payload, unsigned int length);
MQTT client(url, 1883, callback);
void connect(MQTT & client);

// Receive message
void callback(char* topic, byte* payload, unsigned int length) {
  char p[length + 1];
  memcpy(p, payload, length);
  p[length] = NULL;
  
  Serial.println(p);
}


void connect(MQTT & client){
  // connect to the server(unique id by Time.now())
  client.connect(deviceName + "_" + String(Time.now()), MQTTusername, MQTTpassword);
  client.subscribe(topic);
}

// Setup
void setup() {
  Serial.begin(9600);
  Serial.println("Start!");
  
  pinMode(LED, OUTPUT);
  pinMode(Doorbell, INPUT_PULLDOWN);
  
  doorbellDebounced.attach(Doorbell, INPUT_PULLDOWN);
  doorbellDebounced.interval(20);
}

// Loop
void loop() {
  if (client.isConnected()){
    // Loop MQTT
    client.loop();
    
    // Check Doorbell
    if(doorbellDebounced.update()){ // Doorbell changed state
      if(doorbellDebounced.read() == 1){ // Only publish a rising edge
        Serial.println("Doorbell!");
        
        // Publish doorbell
        client.publish(topic, "1");
        
        digitalWrite(LED, HIGH);
        delay(200);
        digitalWrite(LED, LOW);
      }
    }
  }
  else{
    pinMode(Doorbell, OUTPUT);
    digitalWrite(Doorbell, HIGH);
    
    Serial.println("Disconnected! Trying to reconnect now...");
    
    while(!client.isConnected()){
      connect(client);
      delay(500);
    }
    
    // Now connected
    Serial.println("Connected.");
    
    digitalWrite(Doorbell, LOW);
    pinMode(Doorbell, INPUT_PULLDOWN);
  }
}
