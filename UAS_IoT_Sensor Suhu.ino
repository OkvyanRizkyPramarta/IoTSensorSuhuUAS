#include <ESP8266WiFi.h> 
#include <PubSubClient.h> 
#include "DHT.h" 
 
#define DHTTYPE DHT11   // DHT 11 
 
const char* ssid = "Peaberry Coffee & Tea"; 
const char* password = "11333356"; 
 
const char* mqtt_server = "public.mqtthq.com"; 
 
WiFiClient espClient; 
PubSubClient client(espClient); 

const int DHTPin = 5; 
 
const int lamp = 4; 
 
DHT dht(DHTPin, DHTTYPE); 

long now = millis(); 
long lastMeasure = 0; 
 
void setup_wifi() { 
  delay(10); 
  Serial.println(); 
  Serial.print("Connecting to "); 
  Serial.println(ssid); 
  WiFi.begin(ssid, password); 
  while (WiFi.status() != WL_CONNECTED) { 
    delay(500); 
    Serial.print("."); 
  } 
  Serial.println(""); 
  Serial.print("WiFi connected - ESP IP address: "); 
  Serial.println(WiFi.localIP()); 
} 
 
void callback(String topic, byte* message, unsigned int length) { 
  Serial.print("Message arrived on topic: "); 
  Serial.print(topic); 
  Serial.print(". Message: "); 
  String messageTemp; 
   
  for (int i = 0; i < length; i++) { 
    Serial.print((char)message[i]); 
    messageTemp += (char)message[i]; 
  } 
  Serial.println(); 
  
  if(topic=="room/lamp"){ 
      Serial.print("Changing Room lamp to "); 
      if(messageTemp == "on"){ 
        digitalWrite(lamp, HIGH); 
        Serial.print("On"); 
      } 
      else if(messageTemp == "off"){ 
        digitalWrite(lamp, LOW); 
        Serial.print("Off"); 
      } 
  } 
  Serial.println(); 
} 
 
void reconnect() { 
  while (!client.connected()) { 
    Serial.print("Attempting MQTT connection..."); 
    if (client.connect("ESP8266Client")) { 
      Serial.println("connected");   
      client.subscribe("room/lamp"); 
    } else { 
      Serial.print("failed, rc="); 
      Serial.print(client.state()); 
      Serial.println(" try again in 5 seconds"); 
      delay(5000); 
    } 
  } 
} 
 
void setup() { 
  pinMode(lamp, OUTPUT); 
   
  dht.begin(); 
   
  Serial.begin(115200); 
  setup_wifi(); 
  client.setServer(mqtt_server, 1883); 
  client.setCallback(callback); 
 
} 

void loop() { 
 
  if (!client.connected()) { 
    reconnect(); 
  } 
  if(!client.loop()) 
    client.connect("ESP8266Client"); 
 
  now = millis(); 
  if (now - lastMeasure > 30000) { 
    lastMeasure = now; 
    float h = dht.readHumidity(); 
    float t = dht.readTemperature(); 
    float f = dht.readTemperature(true); 
 
    if (isnan(h), isnan(f)) { 
      Serial.println("Failed to read from DHT sensor!"); 
      return; 
    } 
 
    float hic = dht.computeHeatIndex(t, h, false); 
    static char temperatureTemp[7]; 
    dtostrf(hic, 6, 2, temperatureTemp); 
     
    static char humidityTemp[7]; 
    dtostrf(h, 6, 2, humidityTemp); 
 
    client.publish("room/temperature", temperatureTemp); 
    client.publish("room/kelembapan", humidityTemp); 
     
    Serial.print("Humidity: "); 
    Serial.print(h); 
    Serial.print(" %\t Temperature: "); 
    Serial.print(t); 
    Serial.print(" *C "); 
    Serial.print(f); 
    Serial.print(" *F\t Heat index: "); 
    Serial.print(hic); 
    Serial.println(" *C "); 
  } 
}
