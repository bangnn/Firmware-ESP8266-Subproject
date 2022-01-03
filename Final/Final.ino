#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <SoftwareSerial.h>

// WiFi
const char *ssid = "TRUNG - TANG 4"; // Enter your WiFi name
const char *password = "0378474947";  // Enter WiFi password

// MQTT Broker
const char *mqtt_broker = "192.168.0.115";
const char *topicPub = "Station";
const char *topicSub = "client1A";
const char *mqtt_username = "mqtt";
const char *mqtt_password = "123456789";
const int mqtt_port = 1883;

//STM32
#define pinOUT 1 //GPIO 1

//GM65
const int RX = 3; //GPIO 3
const int TX = 2; //GPIO 2


char dataPub[]="";
String dataQR = "M01";
const String idESP = "1A";


WiFiClient espClient;
PubSubClient client(espClient);
SoftwareSerial GM65Serial(RX, TX);

void setup() {
  pinMode(pinOUT, OUTPUT);
   //Mở serial cho stm bắt buộc phải để trên mở serial
  GM65Serial.begin(9600);
  // Set software serial baud to 115200;
  Serial.begin(115200);
  // connecting to a WiFi network
  Serial.print("\nConnecting to WiFi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
  }
  Serial.println("\nConnected to the WiFi network");
  //connecting to a mqtt broker
  client.setServer(mqtt_broker, mqtt_port);
  client.setCallback(callback);
  while (!client.connected()) {
      String client_id = "Client1A";
      client_id += String(WiFi.macAddress());
      if (client.connect("client1A")) {
          Serial.println("Public MQTT broker connected");
      } else {
          Serial.print("failed with state ");
          Serial.println(client.state());
          delay(2000);
      }
  }
}

void callback(char *topicSub, byte *payload, unsigned int length) {
  Serial.print("Message arrived in topic: ");
  Serial.println(topicSub);
  Serial.print("Message:");
  for (int i = 0; i < length; i++) {
    Serial.print((char) payload[i]);
  }

  if((int) payload[0] == 48){
    digitalWrite(pinOUT, LOW);
    Serial.println("\nSend 0 to STM");
  } else {
    digitalWrite(pinOUT, HIGH);
    Serial.println("\nSend 1 to STM");
  }
  Serial.println("-----------------------");
}

void ProcessingDataFromGM65(char dataPub[], String dataQR, String idESP){
  String dataString = dataQR + idESP;
  dataString.toCharArray(dataPub, dataString.length()+1);
  client.publish(topicPub, dataPub);
}

void loop() {
  
  //Nhân tin từ GM65 --> M01
//  while(GM65Serial.available()>0){
//    Serial.println("GM65 is available");
//    dataQR = GM65Serial.read();
//  }
  
  client.loop();
  //Xử lý tin, Gửi về server --> M011A
  if(dataQR.length()>0){ 
    Serial.print("\nMessage from GM65: ");
    Serial.println(dataQR);
    client.subscribe(topicSub);
    ProcessingDataFromGM65(dataPub, dataQR, idESP);
    
  //Nhân phản hồi từ server --> 11A
  //Xử lý tin --> 1
  //Gửi cho STM32 --> 1
    

  //Gửi xong thì xóa mã
    dataQR="";
  }
}
