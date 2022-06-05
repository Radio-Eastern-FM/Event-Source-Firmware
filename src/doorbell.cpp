/******************************************************/
//       THIS IS A GENERATED FILE - DO NOT EDIT       //
/******************************************************/

#include "Particle.h"
#line 1 "d:/DEVELOPMENT/Radio_Eastern/particle_argon/doorbell/src/doorbell.ino"
// Import libraries
#include <MQTT.h>
#include "Debounce/Debounce.h"
#include "DHT.h"

// Allow the code to run before getting internet connection
void setup();
void loop();
void callback(char* recievedTopic, byte* payload, unsigned int length);
#line 7 "d:/DEVELOPMENT/Radio_Eastern/particle_argon/doorbell/src/doorbell.ino"
SYSTEM_THREAD(ENABLED);

///// Pin definitions
// Define the temperature sensor pins
#define DHTPIN 1
#define DHTTYPE DHT22

// Define the doorbell pin
#define DOORBELL D6

// Define the GPIO pins for LEDs
#define BOARD_LED D7
#define BLUE_LED D9

///// Parameter definitions
// Define the parameters for temperature sensor interval
const long unsigned int DHTinterval = 15000;
long unsigned int DHTlastChecked = 0;

// Define MQTT parameters
const String url = "192.168.0.26";
const String topic = "efm/sensors/";
const String deviceName = "MBlashkiArgon";
const String MQTTusername = "MBlashkiArgon";
const String MQTTpassword = "1234";

///// Function Declarations
// Decalare the connect function
void connect(MQTT & client);

// Declare the subscription callback function
void callback(char* topic, byte* payload, unsigned int length);

// Declare utility functions for LEDs
void statusFailure();
void statusSuccess();

// Declare checking functions the sensors
void checkWeather();
void checkDoorbell();

///// Object Initialisers
// Decalare the client
MQTT client(url, 1883, callback);

// Initialise the temperature sensor
DHT dht(DHTPIN, DHTTYPE);

// Initialise the doorbell debouncer
Debounce doorbellDebounced = Debounce();

// Setup
void setup() {
  // Initialise the serial utility
  Serial.begin(9600);
  Serial.println("Start!");
  
  // Begin the temperature sensor
  dht.begin();
  
  // Set Pin modes of the LEDs
  pinMode(BOARD_LED, OUTPUT);
  pinMode(BLUE_LED, OUTPUT);
  pinMode(DOORBELL, INPUT_PULLDOWN);
  
  // Initialise the debounser for the doorbell LED
  doorbellDebounced.attach(DOORBELL, INPUT_PULLDOWN);
  doorbellDebounced.interval(20);
}

// Loop
void loop() {
  if (client.isConnected()){
    // Loop MQTT
    client.loop();
    
    checkDoorbell();
    checkWeather();
  }
  else {
    // Disable doorbell LED
    pinMode(DOORBELL, OUTPUT);
    digitalWrite(DOORBELL, HIGH);
    
    Serial.println("Disconnected! Trying to reconnect now...");
    
    while(!client.isConnected()){
      connect(client);
      delay(500);
      statusFailure();
    }
    
    // Now connected
    Serial.println("Connected.");
    // LED status OK
    statusSuccess();
    
    // Re-enable doorbell LED
    digitalWrite(DOORBELL, LOW);
    pinMode(DOORBELL, INPUT_PULLDOWN);
  }
}

// Receive message
void callback(char* recievedTopic, byte* payload, unsigned int length) {
  char p[length + 1];
  memcpy(p, payload, length);
  p[length] = NULL;
  
  Serial.print(recievedTopic);
  Serial.print(": ");
  Serial.println(p);
  statusSuccess();
}

// Connect to MQTT server and register subscriptions
void connect(MQTT & client){
  // connect to the server(unique id by Time.now())
  client.connect(deviceName + "_" + String(Time.now()), MQTTusername, MQTTpassword);
  // Subscribe to topic so that we know that the device is sending messages
  client.subscribe(topic + "#");
}

void statusFailure(){
  // Strobe LED to show that it's an error
  for (size_t i = 0; i < 3; i++)
  {
    digitalWrite(BLUE_LED, HIGH);
    delay(30);
    digitalWrite(BLUE_LED, LOW);
    delay(60);
  }
}

void statusSuccess(){
  // Flash LED for 75ms
  digitalWrite(BLUE_LED, HIGH);
  digitalWrite(BOARD_LED, HIGH);
  delay(75);
  digitalWrite(BLUE_LED, LOW);
  digitalWrite(BOARD_LED, LOW);
}

void checkWeather(){
  // Check the temperature every DHTinterval milliseconds
  if(DHTlastChecked + DHTinterval <= millis())
  {
    // Read the temperature and humidity
    float hum = dht.readHumidity();
    float temp = dht.readTemperature();
    
    // Check for bad temperature and humidity values
    if (isnan(hum) || isnan(temp))
    {
      // DHT failed to get a reading for some reason.
      Serial.println("Failed to read from DHT sensor!");
      
      // Wait a little time before trying again. The DHT is pretty slow
      delay(1000);
    }
    else{
      // Measurements are valid
      Serial.println("Temperature: " + String(temp) + "    Humidity: " + String(hum));
      // Publish weather
      client.publish(topic + "weather/humidity", String(temp));
      client.publish(topic + "weather/temperature", String(hum));
      
      // Update last checked
      DHTlastChecked = millis();
    }
  }
}

void checkDoorbell(){
  // Check Doorbell
  if(doorbellDebounced.update()){ // Doorbell changed state
    if(doorbellDebounced.read() == 1){ // Only publish a rising edge
      Serial.println("Button pressed.");
      
      // Publish doorbell pressed to the MQTT topic
      client.publish(topic + "doorbell", "1");
    }
  }
}
