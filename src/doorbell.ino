#include <MQTT.h>

const String url = "192.168.0.26";

// Declarations
void callback(char* topic, byte* payload, unsigned int length);
MQTT client(url, 1883, callback);
void connect(MQTT & client);

// Receive message
void callback(char* topic, byte* payload, unsigned int length) {
  char p[length + 1];
  memcpy(p, payload, length);
  p[length] = NULL;
  
  Serial.println("Callback");
  Serial.println(p);
  delay(1000);
}


void connect(MQTT & client){
  // connect to the server(unique id by Time.now())
  client.connect("MBlashkiArgon_" + String(Time.now()), "MBlashkiArgon", "1234");
  client.subscribe("inTopic/message");
}

// Setup
void setup() {
  Serial.begin(9600);
  Serial.println("Start!");
  RGB.control(true);

}

// Loop
void loop() {
  if (client.isConnected()){
    client.loop();
    // publish/subscribe
    Serial.println("Publish");
    client.publish("inTopic/message","hello world");
    delay(30000);
  }
  else{
    Serial.println("Disconnected! Trying to reconnect now...");
    connect(client);
  }
}
