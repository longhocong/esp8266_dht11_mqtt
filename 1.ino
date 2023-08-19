#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>
// WiFi
//const char *ssid = "Redmi Note 8 Pro"; // Enter your WiFi name
// const char *password = "0786799786";  // Enter WiFi password
const char *password = "19139891";
const char *ssid = "MSI";


// MQTT Broker
const char *mqtt_broker = "broker.emqx.io";
const char *topic = "Home/Be1dRoom/DHT22/#";
const char *topicTemp = "Home/Be1dRoom/DHT22/Temperature";
const char *topicHumi = "Home/Be1dRoom/DHT22/Humidity";
const char *mqtt_username = "emqx";
const char *mqtt_password = "public";
const int mqtt_port = 1883;
String sensorID = "Dummy-2";
String receivedData;
char temperature[3];
char humidity[3];
unsigned long i=0;
unsigned long previousMillis = 0;
unsigned long interval = 30000;


#define DHTTYPE DHT11
const int DHTPin = 5;

DHT dht(DHTPin, DHTTYPE);

static char celsiusTemp[7];

static char fahrenheitTemp[7];

static char humidityTemp[7];

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  // Set software serial baud to 115200;
  Serial.begin(115200);
  dht.begin();
  // connecting to a WiFi network
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.println("Connecting to WiFi..");
  }
  WiFi.setAutoReconnect(true);
  WiFi.persistent(true);
  Serial.println("Connected to the WiFi network");
  //connecting to a mqtt broker
  client.setServer(mqtt_broker, mqtt_port);
  client.setCallback(callback);
  while (!client.connected()) {
      String client_id = "esp8266-client-";
      client_id += String(WiFi.macAddress());
      Serial.printf("The client %s connects to the public mqtt broker\n", client_id.c_str());
      if (client.connect(client_id.c_str(), mqtt_username, mqtt_password)) {
          Serial.println("Public emqx mqtt broker connected");
      } else {
          Serial.print("failed with state ");
          Serial.print(client.state());
          delay(2000);
      }
  }
  // publish and subscribe
  //client.publish(topic, "hello emqx");
  //client.subscribe(topic);
}

void callback(char *topic, byte *payload, unsigned int length) {
  Serial.print("Message arrived in topic: ");
  Serial.println(topic);
  Serial.print("Message:");
  for (int i = 0; i < length; i++) {
      Serial.print((char) payload[i]);
  }
  Serial.println();
  Serial.println("-----------------------");
}

void loop() {
  unsigned long currentMillis = millis();
  if ((WiFi.status() != WL_CONNECTED) ) {
    Serial.print(millis());
    Serial.println("Reconnecting to WiFi...");
    WiFi.disconnect();
    //WiFi.begin(ssid, password);
    //Serial.println(WiFi.localIP());
    //Alternatively, you can restart your board
    ESP.restart();
    //Serial.println(WiFi.RSSI());
    previousMillis = currentMillis;
  }
  client.loop();
  if(currentMillis - previousMillis >=10000){
    float h = dht.readHumidity();    
    float t = dht.readTemperature(); 
    Serial.print("Nhiet do: ");
    Serial.println(t);               
    Serial.print("Do am: ");
    Serial.println(h); 
    i=0;
     String jsonStringT = "{\"Sensor_ID\": \"" + sensorID + "\", \"Date\": \""  + "\", \"Temperature\": \"" + t + "\"}";
     const char* payload = jsonStringT.c_str();
     client.publish(topicTemp,payload );
     Serial.println( payload);
      delay(100);
     jsonStringT = "{\"Sensor_ID\": \"" + sensorID + "\", \"Date\": \""  + "\", \"Humidity\": \"" + h + "\"}";
     payload = jsonStringT.c_str();
     client.publish(topicHumi,payload );
     Serial.println( payload);
     previousMillis = currentMillis;
  }
  else i++;

  
  if (Serial.available()) {
    // Đọc dữ liệu nhận được vào biến receivedData
    receivedData = Serial.readString();
    Serial.println(receivedData);
    // Tìm vị trí của ký tự ":" đầu tiên
    int colonIndex = receivedData.indexOf(':');
    Serial.println(colonIndex);
    // Tìm vị trí của ký tự "#" cuối cùng
    int hashIndex = receivedData.lastIndexOf('#');
    Serial.println(hashIndex);
    // Kiểm tra xem chuỗi có chứa ký tự ":" và "#" hay không
    if (colonIndex != -1 && hashIndex != -1 && hashIndex > colonIndex ) {
     // Tách chuỗi con từ vị trí sau ký tự ":" đến trước ký tự "#"
     String Value = receivedData.substring(colonIndex + 6, hashIndex);
     String topicUart = receivedData.substring(colonIndex + 1, colonIndex + 5);
     Serial.println(topicUart);
      if (topicUart=="TEMP"){
      
      Value.toCharArray(temperature, sizeof(temperature));
      //client.publish(topicTemp,temperature );
      String jsonString = "{\"Sensor_ID\": \"" + sensorID + "\", \"Date\": \""  + "\", \"Temperature\": \"" + temperature + "\"}";
      const char* payload = jsonString.c_str();
      client.publish(topicTemp,payload );
      Serial.println( payload);
      }
      else if(topicUart=="HUMI"){
        Value.toCharArray(humidity, sizeof(humidity));
        String jsonString = "{\"Sensor_ID\": \"" + sensorID + "\", \"Date\": \""  + "\", \"Humidity\": \"" + humidity + "\"}";
        const char* payload = jsonString.c_str();
      client.publish(topicHumi,payload );
      }
      
      // Ví dụ: Serial.println(temperature);
    }
  }
}
