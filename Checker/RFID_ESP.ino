#include <ESP8266HTTPClient.h>

#include <MFRC522.h>
#include <SPI.h>

#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>

//#include <WiFiClientSecure.h>

#include <ESP8266WebServer.h>

#define SELECT 14 // This will help in selecting tag registering mode or tags checking (in database) mode

ESP8266WebServer server(80);

/* Network info */
const char *ssid = "Checker";
const char *password = "password";

const char *ssid_1 = "ADOMOU";
const char *password_1 = "alain1671963";

/* Web apps informations */
 String host = "http://www.checker.bj/isAllowed?code=";
 int port = 80;

 char _buffer[30];

#define SS_PIN 5
#define RST_PIN 4
 
MFRC522 mfrc522(SS_PIN, RST_PIN); // Instance of the class

MFRC522::MIFARE_Key key; 

bool state=false;

void setup() { 

  /* Launch Serial communication if useful */
  Serial.begin(9600);

  /* SELECT PIN */
  pinMode(SELECT, INPUT_PULLUP);

  /* Connect to Wireless network */
  WiFi.mode(WIFI_STA);
  WiFi.hostname("CheckWlStation");
  WiFi.begin(ssid, password);

  while(WiFi.status() != WL_CONNECTED){

    delay(200);
  }


  /* mDNS */
  if( !MDNS.begin("checker") ){
    // Is it something to do with this?
  }
  
  SPI.begin(); // Init SPI bus
  mfrc522.PCD_Init(); // Init MFRC522
  

  /* New Tag Query listener */
  server.on("/q", [](){ // Tag registration (on apps database) link 
    if(server.hasArg("tag")){
      if(server.arg("tag") == "0"){ // 0 to ask new tag informations
        if(mfrc522.PICC_IsNewCardPresent()){
          if(mfrc522.PICC_ReadCardSerial()){
            String id = "";
            for(uint8_t i = 0; i < mfrc522.uid.size ; i++){
              id += String(mfrc522.uid.uidByte[i], DEC);
            }
            server.send(200, "text/plain", id);
          }
          
        } else server.send(200, "text/plain", "NACK"); // Not ACKnowledge
      } else server.send(200, "text/plain", "Bad value !!!"); // Minimum Security
    } else server.send(200, "text/plain", "Wrong request !!!"); // 
  });

  server.onNotFound([](){
     server.send(404, "text/plain", "NOT FOUND");
  });

  /* Start http server */
  server.begin();
}

void loop() {

  /* Handle new client request */
  server.handleClient();

   if(!digitalRead(SELECT)){
    /* Create new client object */
     WiFiClient client;
     
     if(mfrc522.PICC_IsNewCardPresent()){
      if(mfrc522.PICC_ReadCardSerial()){
        String id = "";
        for(uint8_t i = 0; i < mfrc522.uid.size ; i++){
          id += String(mfrc522.uid.uidByte[i], DEC);
        }
        Serial.println(id);
        //server.send(200, "text/plain", id);
        delay(200);
        host = "http://www.checker.bj/isAllowed?code=";
        host += id;
        Serial.println(host); 
       host.toCharArray(_buffer, host.length()+1);
        while(client.connect(_buffer, 80)){
          if(client.available()){
            String ans = client.readStringUntil('\n');
            // Update output according to server answer
          }
        }
        client.stop(); 
     }
    }
  }
  delay(100);
}
