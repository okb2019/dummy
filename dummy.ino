#include <Arduino.h>
#include <string.h>
#include <RFM69.h>         //get it here: https://www.github.com/lowpowerlab/rfm69
#include <RFM69_ATC.h>     //get it here: https://www.github.com/lowpowerlab/rfm69
#include <WiFi.h>
#include <PubSubClient.h>
#include <NTPClient.h>
#include <ArduinoJson.h>
#include <ArduinoQueue.h>
#include <AFArray.h>
#include "maindef.h"

struct clientliste {
  char nnc[10];     //netnodeclient numbers
  char topic[128];  //der Topic dazu
  char json[400];  //jason zum Topic
};

//#define DEBUG

//*********************************************************************************************
//************ IMPORTANT SETTINGS - YOU MUST CHANGE/CONFIGURE TO FIT YOUR HARDWARE *************
//*********************************************************************************************
#define NODEID        1    //should always be 1 for a Gateway
#define NETWORKID     100  //the same on all nodes that talk to each other
//Match frequency to the hardware version of the radio on your Moteino (uncomment one):
//#define FREQUENCY     RF69_433MHZ
#define FREQUENCY     RF69_868MHZ
//#define FREQUENCY     RF69_915MHZ
#define ENCRYPTKEY    "sampleEncryptKey" //exactly the same 16 characters/bytes on all nodes!
#define IS_RFM69HW_HCW  //uncomment only for RFM69HW/HCW! Leave out if you have RFM69W/CW!
//*********************************************************************************************
//Auto Transmission Control - dials down transmit power to save battery
//Usually you do not need to always transmit at max output power
//By reducing TX power even a little you save a significant amount of battery power
//This setting enables this gateway to work with remote nodes that have ATC enabled to
//dial their power down to only the required level
#define ENABLE_ATC    //comment out this line to disable AUTO TRANSMISSION CONTROL
//*********************************************************************************************

#define RF69_SLAVE_PIN  5
#define RF69_IRQ_PIN    2

#define SERIAL_BAUD   115200


#define MAXRF69 60

const char* ssid = "Galama";
const char* password = "51579664437547223248";
IPAddress mqtt_server(192,168,178,35);


unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE	(50)
char msg[MSG_BUFFER_SIZE];
int value = 0;

#define MQTT_DEVICE_URL   "homeassistant"
#define MQTT_DATA_URL     "home/OKBGW" 

#define LED_PIN_ERR       25 //Error
#define LED_PIN_RS        26 //RFM69 Send ok
#define LED_PIN_RR        27 //RFM69 Empf ok
#define LED_PIN_MS        32 //MQTT  Send ok
#define LED_PIN_MR        33 //MQTT Empf ok

#define QUEUE_SIZE_ITEMS 20
// Queue creation:
ArduinoQueue<datenspeicher> StructQueue(QUEUE_SIZE_ITEMS);

#ifdef ENABLE_ATC
  RFM69_ATC radio(RF69_SLAVE_PIN,RF69_IRQ_PIN, true);
#elif
  RFM69 radio();
#endif



IPAddress ntpip(192,168,178,1);
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, ntpip);

struct data_transfer my_data;
struct datenspeicher my_datenspeicher;


AFArray<clientliste> myClientList;

void Blink(uint8_t , int);
void callback(char*, byte*, unsigned int);
void setup_wifi(void);
void reconnect(void);
bool Send_Payload(const char*, const char*, const char*);
void process_message_data(datenspeicher&);
void process_message_device(datenspeicher&);
void process_message_request(data_transfer&, uint16_t);
void process_device_update(datenspeicher&);
bool Send_To_MQTT(char*, char*, bool );
bool Send_To_MQTT(char*, char* );
void process_queue(void);

WiFiClient espClient;
PubSubClient client(mqtt_server, 1883, callback, espClient);

void setup() {
  Serial.begin(SERIAL_BAUD);
  delay(1000);
  Serial.println("bla");
  delay(1000);
  if(radio.initialize(FREQUENCY,NODEID,NETWORKID))
    Serial.println("Initializing Radio ..ok");
  else
    Serial.println("Initializing Radio ..failed");
#ifdef IS_RFM69HW_HCW
  radio.setHighPower(); //must include this only for RFM69HW/HCW!
#endif
  radio.encrypt(ENCRYPTKEY);
  char buff[50];
  sprintf(buff, "\nTransmitting at %d Mhz...", FREQUENCY==RF69_433MHZ ? 433 : FREQUENCY==RF69_868MHZ ? 868 : 915);
  Serial.println(buff);


  pinMode(LED_PIN_ERR, OUTPUT);
  pinMode(LED_PIN_RS, OUTPUT);
  pinMode(LED_PIN_RR, OUTPUT);
  pinMode(LED_PIN_MS, OUTPUT);
  pinMode(LED_PIN_MR, OUTPUT);
  
  delay(1000);

  setup_wifi();
}

void loop() {
  
  if (!client.connected()) {
    reconnect();
  }
  
  if(radio.receiveDone())
  {
    Process_RFM69_Message();
  }  
  else
    process_queue();
  client.loop();
}

void Blink(byte PIN, int DELAY_MS)
{
  digitalWrite(PIN,HIGH);
  delay(DELAY_MS);
  digitalWrite(PIN,LOW);
}

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  char buf[50];
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client";
    //clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str(), "olaf", "finnibu2000")) {
      Serial.println("connected");
      client.setBufferSize(512);   // default sind 256, das ist zu klein für die Payloads.
      // Once connected, publish an announcement...
      client.publish("outTopic", "hello world");
      // ... and resubscribe
      snprintf(buf,49, "%s/+/cmd",MQTT_DATA_URL);
      client.subscribe(buf);
      Serial.print("Sujbscribed To : ");
      Serial.println(buf);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}


void callback(char* topic, byte* payload, unsigned int length) {
  
  char node[4];
  char client[4];
  char sendung[50];
  int buflen;
  for (int z = 0; z < 3; z++){
     node[z] = topic[strlen(MQTT_DATA_URL) + z + 4];
     client[z] = topic[strlen(MQTT_DATA_URL) + z + 7];
  }
  node[3] = '\0';
  client[3] = '\0';
  
 
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");
    for (unsigned int i = 0; i < length; i++) {
      Serial.print((char)payload[i]);
    }
    Serial.println();

    Serial.println(node);
    Serial.println(client);
    Serial.println(atoi(node));
 
  // sendendata.client_id = atoi(client);
  if(length > 49)
    buflen =50;
  else
    buflen = length;
  for (unsigned int i = 0; i < buflen; i++) {
      sendung[i] = (char)payload[i];
    }
  sendung[buflen] = '\0';
  
  Serial.println(sendung);

  Blink(LED_PIN_MR,5);   
  
  if (Send_Payload(node, client, sendung)){
  //if (Send_Payload(1, my_data)){  
    Serial.println("RF69  send ok!");
    Blink(LED_PIN_RS,5);
  }
  else {
    Serial.println("RF69 Send nok...");
    Blink(LED_PIN_ERR,5);
  }

}

bool Send_Payload(const char *des_id, const char* clid, const char* pl)
{
  my_data.client_id = (uint8_t)atoi(clid);
  my_data.command = C_SET;
  my_data.type = 0;
  strncpy(my_data.payload,pl, 58);
  if (radio.sendWithRetry((uint16_t)atoi(des_id), (const void*)(&my_data), sizeof(my_data)))
      {
        Serial.println("RF69  send ok!");
        Blink(LED_PIN_RS,5);
        return true;
      }
      else 
      {
        Serial.println("RF69 Send nok...");
        Blink(LED_PIN_ERR,5);
        return false;
      }
}

void Process_RFM69_Message()
{
  uint16_t sender_id;
  data_transfer dummy;
  datenspeicher datendummy;
  Serial.println("checking radio...");
  if (radio.DATALEN != sizeof(data_transfer))
  {
    Serial.print("Invalid payload received, not matching Payload struct!");
    Blink(LED_PIN_ERR, 5);
  }
  else
  {
    // *(data_transfer*)radio.DATA
    Serial.print(" [RX_RSSI:");Serial.print(radio.readRSSI());Serial.print("]");
    datendummy.sender_id = radio.SENDERID;
    datendummy.paket = *(data_transfer*)radio.DATA;
    datendummy.RSSI = radio.readRSSI();
    if (!StructQueue.isFull()) {
    #ifdef DEBUG  
      Serial.printf("Adding value from : %d %d ", radio.SENDERID, datendummy.paket.client_id);
    #endif
      StructQueue.enqueue(datendummy);
      if (radio.ACKRequested())
      {
        radio.sendACK();
      //  Serial.println(" - ACK sent");
        delay(10);
      }
    }  
    else  
      Serial.println();
    Blink(LED_PIN_RS, 5);
  }
  /*
  #ifdef DEBUG
    Serial.print(sender_id);
    Serial.print(" - ");
    Serial.print(dummy.client_id);
    Serial.print(" - ");
    Serial.print(dummy.command);
    Serial.print(" - ");
    Serial.print(dummy.type);
    Serial.print(" - ");
    //Serial.print(my_datenspeicher.RSSI);
    Serial.print(" - ");
    Serial.println(dummy.payload);
  #endif  
  */
}

void process_queue(void)
{
  datenspeicher datendummy;
  if(!StructQueue.isEmpty())
  {
    datendummy = StructQueue.dequeue();
    switch(datendummy.paket.command)
    {
      case C_SET:
        process_message_data(datendummy);
        break;
      case C_PRESENTATION:
        process_message_device(datendummy);
        break;
      case C_REQ:
        //process_message_request();
        break;
      case C_UPDATE:
      process_device_update(datendummy);
        break;
      default:
        break;
    }
  }
}

bool Send_To_MQTT(char *topic, char *payload, bool statisch){

  if(client.publish(topic, payload, statisch)){
    //Serial.println("MQTT - ok.");
    Blink(LED_PIN_MS,5);
    return true;
  }  
  else{  
    Serial.println("MQTT - FEHLER");
    
    Serial.print(strlen(topic));
    Serial.println(topic);
    
    Serial.print(strlen(payload));
    Serial.println(payload);
    Blink(LED_PIN_ERR,5);
    return false;
  }
}

bool Send_To_MQTT(char *topic, char *payload ){

  if(client.publish(topic, payload)){
  //  Serial.println("MQTT - ok.");
    Blink(LED_PIN_MS,5);
    return true;
  }  
  else{  
    Serial.println("MQTT - FEHLER");
    Serial.println(topic);
    Serial.println(payload);
    Blink(LED_PIN_ERR,5);
    return false;
  }
}


void process_message_data(datenspeicher &data)
{
  char topicbuffer[128]; // fuer MQTT Topic   
  char myonline[] = "online";
  uint8_t merker = -1;
 
  snprintf(topicbuffer, 128,"%s/%03d%03d%03d/stat", MQTT_DATA_URL,NETWORKID,data.sender_id,data.paket.client_id);
  
  #ifdef DEBUG
    Serial.print(topicbuffer);
    Serial.print(" - ");
    Serial.println(data.paket.payload);
  #endif
    Send_To_MQTT(topicbuffer, data.paket.payload);

// und avail auf online setzten
  snprintf(topicbuffer, 128,"%s/%03d%03d%03d/avail", MQTT_DATA_URL,NETWORKID,data.sender_id,data.paket.client_id);
  
  Send_To_MQTT(topicbuffer, myonline );

// jetzt noch die Signalstärke
  data.paket.client_id = 99;   // 99 ist immer der Sensor für die Signalstärke
  Blink(LED_PIN_RR,3);
  snprintf(topicbuffer, 50,"%s/%03d%03d%03d/stat", MQTT_DATA_URL,NETWORKID,data.sender_id,data.paket.client_id);
  snprintf(data.paket.payload,58,"%d",data.RSSI);
  Send_To_MQTT(topicbuffer, data.paket.payload);

// auch die Signalstärke braucht ein avail
  snprintf(topicbuffer, 50,"%s/%03d%03d%03d/avail", MQTT_DATA_URL,NETWORKID,data.sender_id,data.paket.client_id);
  Send_To_MQTT(topicbuffer, myonline );
}


void process_message_device(datenspeicher &data)
{
  char buffer[500];       // fuer das MQTT Payload
  char topicbuffer[128];  // fuer MQTT Topic  
  char subdatatopic[10];      // fuer den festen Bestandteil
  clientliste clarray;
  char *clname;
  char *clnameteil2;
  char *clnameteil3;
  int indexnummer = -1;
  int lauf = 0;

  clname = strtok(data.paket.payload,";");
  clnameteil2 = strtok(NULL,";");               //nodename bei dem Node selber, oder min wert bei dem Client
  clnameteil3 = strtok(NULL, ";");              //version bei dem Node selber, oder max wert bei dem Client

  StaticJsonDocument<400>  JSONbuffer;                  // Daten String zum senden den den MQTT Server 
  
  // create an object and subobject
  JsonObject mqtt_buffer = JSONbuffer.to<JsonObject>();
  JsonObject rfm69device = mqtt_buffer.createNestedObject("dev");
  JsonArray rfm69ident = rfm69device.createNestedArray("identifiers");
  //ist bei vielen gleich
  snprintf(subdatatopic, 10, "%03d%03d%03d", NETWORKID, data.sender_id, data.paket.client_id );


  //Das Topic zusammenbauen
  snprintf(topicbuffer, 50, "%s/%s/%s/config",MQTT_DEVICE_URL,device_type[data.paket.type].name, subdatatopic);
  
  #ifdef DEBUG
    Serial.println(topicbuffer);   
  #endif

  mqtt_buffer["name"] = clname;
  
  snprintf(buffer, 50, "%s/%s/stat", MQTT_DATA_URL,subdatatopic);
  mqtt_buffer["stat_t"] = buffer;
  
  snprintf(buffer, 50, "%s/%s/avail", MQTT_DATA_URL,subdatatopic);
  mqtt_buffer["avty_t"] = buffer;

  mqtt_buffer["pl_avail"] =  "online";
  mqtt_buffer["pl_not_avail"] = "offline";

  //snprintf(buffer, 50, "{{ value_json.%s}}", clinfo1.val_tpl);
  //mqtt_buffer["val_tpl"] = buffer;
  //if((dummy.paket.type != 13) || (dummy.paket.type != 14)){  // Text und Cover haben keine device_class
  //  mqtt_buffer["dev_cl"] = dev_cla[clinfo1.dev_cl].name;
  //}

  snprintf(buffer, 50, "%.5s%s",device_type[data.paket.type].name,subdatatopic);
  mqtt_buffer["unique_id"] = buffer;
  
  if(data.paket.type != S_SENSOR ){ // Sensoren haben kein cmd
    
    snprintf(buffer, 50, "%s/%s/cmd", MQTT_DATA_URL,subdatatopic);
    mqtt_buffer["cmd_t"] = buffer;
  }
  else {
    #ifdef DEBUG
      Serial.println(" Kein CMD ");
    #endif
  }

  
/*
  if(clinfo1.sta_cl == 14) // Cover haben ein position_topic
  {
    snprintf(buffer, 50, "%s/%03d%03d%03d/position", MQTT_DATA_URL,NETWORKID, senderid,clinfo1.client_id);
    mqtt_buffer["pos"] = buffer;
  }
*/
  //if(clinfo1.unit_of_m >= 0)  
  //  mqtt_buffer["unit_of_meas"] = unit_of_meas[clinfo1.unit_of_m].name;
  
  rfm69device["name"] = clnameteil2;
  snprintf(buffer,50,"%.5s%s",clnameteil2,subdatatopic);
  rfm69ident.add(buffer);
  
  serializeJson(mqtt_buffer, buffer);
  Send_To_MQTT(topicbuffer, buffer, true); 

  #ifdef DEBUG
    Serial.println(topicbuffer);
    serializeJsonPretty(mqtt_buffer, Serial);
  #endif
  Blink(LED_PIN_RR,3);

  while (indexnummer == -1 && lauf < myClientList.size())
  {
    clarray = myClientList[lauf];
    if(strcmp(clarray.nnc, subdatatopic) == 0)
      indexnummer = lauf;
  }

  if(indexnummer == -1)
  {
    strcpy(clarray.nnc, subdatatopic);
    strcpy(clarray.topic, topicbuffer);
    strcpy(clarray.json, buffer);
    myClientList.add(clarray);
    JsonDocument doc;
    deserializeJson(doc, buffer);
    Serial.printf("Neues Element:\n");
    serializeJsonPretty(doc, Serial);
  }
}

void process_device_update(datendummy &data)
{
  char buffer[500];       // fuer das MQTT Payload
  char topicbuffer[128];  // fuer MQTT Topic  
  char subdatatopic[10];      // fuer den festen Bestandteil
  clientliste clarray;
  char *clname;
  char *clwert;
  int indexnummer = -1;
  int lauf = 0;

  clname = strtok(data.paket.payload,";");  //Was soll hinzugefügt werden
  clwert = strtok(NULL,";");                //der Wert dazu
  

  JsonDocument jsondoc;                       // Daten String zum senden den den MQTT Server 

  snprintf(subdatatopic, 10, "%03d%03d%03d", NETWORKID, data.sender_id, data.paket.client_id );  
  while (indexnummer == -1 && lauf < myClientList.size())
  {
    clarray = myClientList[lauf];
    if(strcmp(clarray.nnc, subdatatopic) == 0)
      indexnummer = lauf;
  }
  if(indexnummer == -1)
  {
    strcpy(subdatatopic, clarray.nnc);
    strcpy(topicbuffer, clarray.topic);
    strcpy(buffer, clarray.json);
    deserializeJson(jsondoc, buffer);
    serializeJsonPretty(doc, Serial);
  }
  jsondoc[clname] = clwert;
  serializeJson(jsondoc, buffer);
  strncpy(clarray.json, buffer, 400);
  Send_To_MQTT(topicbuffer, buffer, true); 
  myClientList[lauf] = clarry;

void process_message_request(data_transfer &data, uint16_t sender_id)
{

}
