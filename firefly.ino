// Select your modem:
//define TINY_GSM_MODEM_SIM800
#define TINY_GSM_MODEM_SIM900
// #define TINY_GSM_MODEM_SIM808
// #define TINY_GSM_MODEM_SIM868
// #define TINY_GSM_MODEM_SIM7000
// #define TINY_GSM_MODEM_UBLOX
// #define TINY_GSM_MODEM_M95
// #define TINY_GSM_MODEM_BG96
// #define TINY_GSM_MODEM_A6
// #define TINY_GSM_MODEM_A7
// #define TINY_GSM_MODEM_M590
// #define TINY_GSM_MODEM_MC60
// #define TINY_GSM_MODEM_MC60E
// #define TINY_GSM_MODEM_ESP8266
// #define TINY_GSM_MODEM_XBEE

#include <TinyGsmClient.h>
#include <PubSubClient.h>

// Set serial for debug console (to the Serial Monitor, default speed 115200)
#define SerialMon Serial

// Use Hardware Serial on Mega, Leonardo, Micro
//#define SerialAT Serial1

// or Software Serial on Uno, Nano
#include <SoftwareSerial.h>
SoftwareSerial SerialAT(10, 11); // RX, TX


// Your GPRS credentials
// Leave empty, if missing user or pass
const char apn[]  = "APN_NAME";
const char user[] = "";
const char pass[] = "";

//MQTT credentials
const char* mqttusername = "yourmqttusername"; //MQTT USER CREDENTIALS from Device Onboarding Page

const char* mqttpassword = "yourpassword"; //MQTT USER CREDENTIALS from Device Onboarding Page

String DEVICE_SERIAL = "mac_address"; //Serailno of the device from Device Operation page

//Backend credentials

const char* mqtt_server = "broker"; //mqtt.exampledomain.com

char* EVENT_TOPIC = "topik"; //EVENT Topic from Device Onboarding Page

String SUB_TOPIC_STRING = "sub"; //Action Topic from Device Onboarding Page

String PUB_TOPIC_STRING = "pub"; //Action Topic Response from Device Onboarding Page

char msg[1024];


TinyGsm modem(SerialAT);
TinyGsmClient client(modem);
PubSubClient mqtt(client);

long lastReconnectAttempt = 0;
int distance=5;

void setupGSM() {
  
  // Restart takes quite some time
  // To skip it, call init() instead of restart()
  SerialMon.println("Initializing modem...");
  modem.restart();

  String modemInfo = modem.getModemInfo();
  SerialMon.print("Modem: ");
  SerialMon.println(modemInfo);
  
  SerialAT.println("AT+CIPMUX=0");
  delay(2000);
  printSerialData();

  SerialAT.println("AT+CGATT?");
  delay(2000);
  printSerialData();

  SerialAT.println("AT+CSTT=\"apn\",\"\",\"\"");

  SerialAT.println("AT+COPS?");
  delay(2000);
  printSerialData();

  SerialMon.print("Connecting to ");
  SerialMon.print(apn);
  if (!modem.gprsConnect(apn, user, pass)) {
    SerialMon.println(" fail");
    while (true);
  }
 
  SerialMon.print("Waiting for network...");
  if (!modem.waitForNetwork()) {
    SerialMon.println(" fail");
    while (true);
  }
  SerialMon.println(" OK");

  
  SerialMon.println(" OK");
}

void setup(){
  // Set console baud rate
  SerialMon.begin(9600);
  delay(10);

  // Set GSM module baud rate
  SerialAT.begin(9600);
  delay(3000);

  //Setup GPRS
  setupGSM();

  // MQTT Broker setup
  mqtt.setServer(mqtt_server, 1883);
  
}

void printSerialData()
{
 while(SerialAT.available()!=0)
 Serial.write(SerialAT.read());
}

void reconnect() {

  // Loop until we're reconnected

  while (!mqtt.connected()) {

    Serial.print("Attempting MQTT connection...");

    // Create a random client ID

    String clientId = "UnoClient-";

    clientId += String(random(0xffff), HEX);

    // Attempt to connect

    if (mqtt.connect(clientId.c_str(), "topik", "topik_1")) {

      Serial.println("connected");

      //subscribe to the topic

      const char* SUB_TOPIC = SUB_TOPIC_STRING.c_str();

      mqtt.subscribe(SUB_TOPIC);

      } else {

      Serial.print("failed, rc=");

      Serial.print(mqtt.state());

      Serial.println(" try again in 5 seconds");

      // Wait 5 seconds before retrying

      delay(5000);

        }

      }

    }

  void publish_message(const char* message) {

  Serial.println(message); //print to the console

  mqtt.publish(EVENT_TOPIC, message); //post to the Event topic on the server
  
  }

  void readSensorValues(){
  
    String message = "{\"eventName\":\"dataChanged\",\"status\":\"none\",\"ph\":\""+ String(pHValue) +"\",\"turbidity\":\""+ String(turbid) +"\",\"light\":\""+ String(light) +"\",\"mac\":\"mac_address\"}";  

    char* msg = (char*)message.c_str();

    publish_message(msg); //send the event to backend

  }

  void loop() {

  if (!mqtt.connected()) {
    SerialMon.println("=== MQTT NOT CONNECTED ===");
    // Reconnect every 10 seconds
    reconnect();
  }
  readSensorValues();
  delay(4000);
  mqtt.loop();
  }
