
// MQTT client (esp32)



#include <PubSubClient.h>  // MQTT clients
#include <NewPing.h> // read data Ultrsounic sensor
#include <Wire.h>
#include <WiFi.h>


// Replace the next variables with your SSID/Password combination
const char* ssid = "REPLACE_WITH_YOUR_SSID";
const char* password = "REPLACE_WITH_YOUR_PASSWORD";

// Add your MQTT Broker IP address, example:
//const char* mqtt_server = "192.168.1.144";
const char* mqtt_server = "YOUR_MQTT_BROKER_IP_ADDRESS";

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

/////////////////////////////////////////////////////
#define TRIGGER_PIN 2 // pin tied to trigger pin on the ultrasonic sensor.
#define ECHO_PIN 4 // pin tied to echo pin on the ultrasonic sensor.
#define MAX_DISTANCE 50 // Maximum distance we want to ping for (in centimeters). Maximum sensor
NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE); // NewPing setup of pins and maximum
int distance; // store reading of Ultrasonic sensor

#define ledPin 5 // pin connect to led

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);

  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  pinMode(ledPin, OUTPUT);

}

void loop() {
  // put your main code here, to run repeatedly:

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  long now = millis();
  if (now - lastMsg > 5000) {
    lastMsg = now;
    
    // distance from obstical in cm
    int time = sonar.ping_median(5); //median off 5 values_ and it returen the time in microseconds
    distance = sonar.convert_cm(time); //convert that to cm, replace "cm" with "in" for inches
    Serial.print("distance: ");
    Serial.print(distance); // //print value to screen so we can see it.
    Serial.println(" cm"); 
   
    if ( digitalRead(ledPin)==HIGH){
    // Convert the value to a char array
    char distance_string[8];
    dtostrf(distance, 1, 2, distance_string);
    Serial.print("distance: ");
    Serial.println(distance_string);
    client.publish("esp32/distance", distance_string);
    }
  }
}


void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String message_string;
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
     message_string += (char)message[i];
  }
  Serial.println();

  // Feel free to add more if statements to control more GPIOs with MQTT

  // If a message is received on the topic esp32/output, you check if the message is either "on" or "off". 
  // Changes the output state according to the message
  if (String(topic) == "esp32/led") {
    Serial.print("Changing output to ");
    if( message_string == "on"){
      Serial.println("on");
      digitalWrite(ledPin, HIGH);
    }
    else if( message_string == "off"){
      Serial.println("off");
      digitalWrite(ledPin, LOW);
    }
  }
}


void reconnect() {            // Subscribe to topics
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client")) {
      Serial.println("connected");
      // Subscribe
      client.subscribe("esp32/led");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}



