////////////////////////////////////////////////////////////
// 		    Code writen by: bioshock2k  4/13/2020         //
//	Github repository:  bioshock2k /ESP32_House_monitoring//
////////////////////////////////////////////////////////////

#include <WiFi.h>
#include <PubSubClient.h>

//-----Librarys
#include <EmonLib.h>
#include "chewing.h"
#include "wifi_setup.h"

//-----Pins
//Relays
#define switch1 27	//3 relays i´ve used
#define switch2 14
#define switch3 12
//Current Sensors
#define SENSOR0 33 // Set the GPIO33 as SENSOR
#define SENSOR1 32// Set the GPIO12 as SENSOR
#define SENSOR2 35 // Set the GPIO12 as SENSOR

#define led 2		//inbuilt led on ESP32

#define sensor1_offset 0.58
#define sensor2_offset 0.52
#define sensor3_offset 0.61

//Defining Emonlib for 3 phases
EnergyMonitor emon1;        
EnergyMonitor emon2; 
EnergyMonitor emon3;    
//Other librarys
filter chew;
wf wf;        //WIFI library


float temperature = 0;

/* create an instance of PubSubClient client */
WiFiClient espClient;
PubSubClient client(espClient);

/* topics */
#define TEMP_TOPIC    "garage/temp"
#define C1_TOPIC    "garage/current1"
#define C2_TOPIC    "garage/current2"
#define C3_TOPIC    "garage/current3"
#define C4_TOPIC    "garage/current4"
#define SWITCH1_TOPIC     "garage/1switch" /* true=on, false=off */
#define SWITCH2_TOPIC     "garage/2switch" /* true=on, false=off */
#define SWITCH3_TOPIC     "garage/3switch" /* true=on, false=off */


long lastMsg = 0;
char msg[20];

float CONTROL1; // Name of the variable to be used within the code.
float CONTROL2; // Name of the variable to be used within the code.
float CONTROL3; // Name of the variable to be used within the code.

int led_c=0;
// Force EmonLib to use 10bit ADC resolution
#define ADC_BITS    10
#define ADC_COUNTS  (1<<ADC_BITS)

void receivedCallback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message received: ");
  Serial.println(topic);

  if (topic[7]=='1')
  {
    Serial.print("payload: ");
    for (int i = 0; i < length; i++) {
      Serial.print((char)payload[i]);
    }
    Serial.println();
    /* we got '1' -> on */ 
    if ((char)payload[0] == 't') {
      led_c=1;
      digitalWrite(switch1, HIGH);
    } else {
     /* we got '0' -> on */
     led_c=0;
     digitalWrite(switch1, LOW);
    }
  }
  if (topic[7]=='2')
  {
    Serial.print("payload: ");
    for (int i = 0; i < length; i++) {
      Serial.print((char)payload[i]);
    }
    Serial.println();
    /* we got '1' -> on */
    if ((char)payload[0] == 't') {
      digitalWrite(switch2, HIGH); 
    } else {
     /* we got '0' -> on */
     digitalWrite(switch2, LOW);
    }
  }
  if (topic[7]=='3')
  {
    Serial.print("payload: ");
    for (int i = 0; i < length; i++) {
      Serial.print((char)payload[i]);
    }
    Serial.println();
    /* we got '1' -> on */
    if ((char)payload[0] == 't') {
      digitalWrite(switch3, HIGH); 
    } else {
     /* we got '0' -> on */
     digitalWrite(switch3, LOW);
    }
  }
}

void mqttconnect() {
  /* Loop until reconnected */
   int trys=0;
  while (!client.connected()) {
    Serial.print("MQTT connecting ...");
    /* client ID */
    String clientId = "ESP32Client";
    /* connect now */
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      trys=0;
      /* subscribe topic with default QoS 0*/
      client.subscribe(SWITCH1_TOPIC);
      client.subscribe(SWITCH2_TOPIC);
      client.subscribe(SWITCH3_TOPIC);
    } 
     if( trys>20){
    ESP.restart();
    trys=0;
    }
    else {
      trys++;
      Serial.print("failed, status code =");
      Serial.print(client.state());
      Serial.println("try again in 5 seconds");
      /* Wait 5 seconds before retrying */
      delay(3000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  analogReadResolution(ADC_BITS);                               //Enable 10 bits in emonlib
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  pinMode(SENSOR0, INPUT);
  pinMode(SENSOR1, INPUT);
  pinMode(SENSOR2, INPUT);
  
  pinMode(switch1, OUTPUT);
  pinMode(switch2, OUTPUT);
  pinMode(switch3, OUTPUT);
  pinMode(led, OUTPUT);

  emon1.current(SENSOR0,30);
  emon2.current(SENSOR1,30); 
  emon3.current(SENSOR2,30);  
  
  // Configure and connect to WIFI
  wf.all();

  /* configure the MQTT server with IPaddress and port */
  client.setServer(mqtt_server, 1883);
  /* this receivedCallback function will be invoked 
  when client received subscribed topic */
  client.setCallback(receivedCallback);
}

void loop() {
  /* if client was disconnected then try to reconnect again */
  if (!client.connected()) {
    mqttconnect();
  }
  /* this function will listen for incomming 
  subscribed topic-process-invoke receivedCallback */
  client.loop();
  /* we measure temperature every 3 secs
  we count until 3 secs reached to avoid blocking program if using delay()*/
  long now = millis();
  if (now - lastMsg > 1000) {
    temperature = random(0, 40);
    lastMsg = now;
     //------Current Sensor value--------
    double sensor1 = emon1.calcIrms(1480);
    double sensor2 = emon2.calcIrms(1480);
    double sensor3 = emon3.calcIrms(1480);
    

  // -------Offset setting------------- 
    sensor1 = sensor1 - sensor1_offset;
    sensor2 = sensor2 - sensor2_offset;
    sensor3 = sensor3 - sensor3_offset;
  //---------FILTERTING----------------
    sensor1 = chew.chewing_it0(sensor1);                   //FILTERTING
    sensor2 = chew.chewing_it1(sensor2);
    sensor3 = chew.chewing_it2(sensor3);

    //Serial.println(sensor1);
    if (!isnan(temperature)) {
      snprintf (msg, 20, "%lf", temperature);
      /* publish the message */
      client.publish(TEMP_TOPIC, msg);
      
      snprintf (msg, 20, "%lf", sensor1);
      client.publish(C1_TOPIC, msg);

      snprintf (msg, 20, "%lf", sensor2);
      client.publish(C2_TOPIC, msg);

      snprintf (msg, 20, "%lf", sensor3);
      client.publish(C3_TOPIC, msg);
    }
  }
}
