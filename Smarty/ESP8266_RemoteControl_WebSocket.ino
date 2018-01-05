#include <Arduino.h>

#include <ESP8266WiFi.h>
//#include <ESP8266WiFiMulti.h>
#include <ESP8266WebServer.h>
#include <WebSocketsServer.h>
#include <Hash.h>

/* Device */
#define gpio_1_ 5
#define gpio_2_ 4

/* Network info */
const char *ssid = "Smarty";
const char *password = "password";
/*
const char *ssid_c = "authentic";
const char *password_c = "9876543210";
*/

//ESP8266WiFiMulti WiFiMulti;

ESP8266WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);

uint8_t gpio[10] = {5, 4, 14, 12};

void setup(){
  WiFi.softAP(ssid, password);

  for(uint8_t i = 0; i < 4; i++){
    pinMode(gpio[i], OUTPUT);
  }

  server.on("/dev", [](){
    if(server.hasArg("gpio")){
      if(server.arg("gpio").toInt() < 4){
        digitalWrite(gpio[server.arg("gpio").toInt()], server.arg("state").toInt());
        server.send(200, "text/html", "OK");
      }else { server.send(200, "text/html", "NO"); }
      delay(500);
    }
  });

  server.on("/synch",[](){
    if(server.hasArg("gpio")){
      int request = server.arg("gpio").toInt();
      String response = ":";
      response += (char)(digitalRead(request)+48); // As ASCII of char 0 = 48
      response += ':';
      server.send(200, "text/html", response);
    }
  });

  server.on("/sensor", [](){
    if(server.hasArg("gpio") && server.arg("gpio").toInt() == 0){
      int value = analogRead(server.arg("gpio").toInt());
      server.send(200, "text/html", String(value));
    }
  });
/*
  WiFiMulti.addAP(ssid, password);

  while(WiFiMulti.run() != WL_CONNECTED) {
      delay(100);
  }
*/
  server.begin();

  /* Start Web Socket server */
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
  delay(100);
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t* payload, size_t len){
  int pin = 0, val = 0;
  switch(type){
    case WStype_DISCONNECTED:{

      break;
    }
    case WStype_CONNECTED:{
      
      break;
    }
    case WStype_TEXT:{
      // New message received
      char _buf[20];
      for(int i = 0; i < len ; i++){
          _buf[i] = (char)payload[i];
      }
      String str = String(_buf);
      if(payload[0] == 's'){
        //sscanf((const char*)&payload, "s:%d:%d:", &pin, &val);
        
        pin = str.substring(str.indexOf(":")+1,str.indexOf(":",2)).toInt();
        val = str.substring(str.indexOf(":", 3)+1,str.indexOf(":",4)).toInt();
        digitalWrite(gpio[pin], val);
        webSocket.sendTXT(num, "OK");
        delay(100);
      }else if(payload[0] == 'r'){
        String str = String((const char*)&payload);
        pin = str.substring(str.indexOf(":")+1,str.indexOf(":",2)).toInt();
        int value = analogRead(pin);
        char _buffer[10];
        sprintf(_buffer, "%4d\0", value);
        
        webSocket.sendTXT(num, _buffer);
      }else if(payload[0] == 'q'){
        String str = String((const char*)&payload);
        pin = str.substring(str.indexOf(":")+1,str.indexOf(":",2)).toInt();
        int value = digitalRead(pin);
        char _buffer[5];
        sprintf(_buffer, "%1d\0", value);
        
        webSocket.sendTXT(num, _buffer);
      }
      break;
    }
  }
}

void loop(){
  webSocket.loop();
  server.handleClient();
}

