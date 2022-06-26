/*
  Base on 
  Simple.ino, Example for the AutoConnect library.
  Copyright (c) 2018, Hieromon Ikasamo
  https://github.com/Hieromon/AutoConnect
  
  This software is released under the MIT License.
  https://opensource.org/licenses/MIT
*/
/*
  Base on HandlePortal.ino, Example for the AutoConnect library.
  Copyright (c) 2018, Hieromon Ikasamo
  https://github.com/Hieromon/AutoConnect

  This software is released under the MIT License.
  https://opensource.org/licenses/MIT
*/
/*
  This is a way of not explicitly declaring ESP8266WebServer. It uses
  the ESP8266WebServer function without its declaration.
  I recommend that you consider this example compared to HandlePortalEX.ino.
  https://github.com/Hieromon/AutoConnect/blob/master/examples/HandlePortalEX/HandlePortalEX.ino
  It will help you understand AutoConnect usage.
*/

/*
  esp8266 server control pin D0-4 wifi autoconnect
  scan wifi and config SSID and pasword 
  shows server ip address for on off switch
  No need to open Serial monitor  
 
*/
 

#if defined(ARDUINO_ARCH_ESP8266)
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#elif defined(ARDUINO_ARCH_ESP32)
#include <WiFi.h>
#include <WebServer.h>
#endif

#include <AutoConnect.h>
#define RELAY_NO    true
#define NUM_RELAYS  5

// Assign each GPIO to a relay
int relayGPIOs[NUM_RELAYS] = {16, 5 , 4, 0, 2};


#if defined(ARDUINO_ARCH_ESP8266)
ESP8266WebServer Server;
#elif defined(ARDUINO_ARCH_ESP32)
WebServer Server;
#endif

AutoConnect       Portal(Server);
AutoConnectConfig Config;       // Enable autoReconnect supported on v0.9.4

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    html {font-family: Arial; display: inline-block; text-align: center;}
    h2 {font-size: 3.0rem;}
    p {font-size: 3.0rem;}
    body {max-width: 600px; margin:0px auto; padding-bottom: 25px;}
    .switch {position: relative; display: inline-block; width: 120px; height: 68px} 
    .switch input {display: none}
    .slider {position: absolute; top: 0; left: 0; right: 0; bottom: 0; background-color: #ccc; border-radius: 34px}
    .slider:before {position: absolute; content: ""; height: 52px; width: 52px; left: 8px; bottom: 8px; background-color: #fff; -webkit-transition: .4s; transition: .4s; border-radius: 68px}
    input:checked+.slider {background-color: #2196F3}
    input:checked+.slider:before {-webkit-transform: translateX(52px); -ms-transform: translateX(52px); transform: translateX(52px)}
  </style>
</head>
<body>
  <h2>ESP Web Server</h2>
  %BUTTONPLACEHOLDER%
<script>function toggleCheckbox(element) {
  var xhr = new XMLHttpRequest();
  if(element.checked){ xhr.open("GET", "/update?relay="+element.id+"&state=1", true); }
  else { xhr.open("GET", "/update?relay="+element.id+"&state=0", true); }
  xhr.send();
}</script>
</body>
</html>
)rawliteral";

String processor1(const String& var){
  //Serial.println(var);
  if(var == "BUTTONPLACEHOLDER"){
    String buttons ="";
    for(int i=1; i<=NUM_RELAYS; i++){
      String relayStateValue = relayState(i);
      buttons+= "<h4>Relay #" + String(i) + " - GPIO " + relayGPIOs[i-1] + "</h4><label class=\"switch\"><input type=\"checkbox\" onchange=\"toggleCheckbox(this)\" id=\"" + String(i) + "\" "+ relayStateValue +"><span class=\"slider\"></span></label>";
    }
    return buttons;
  }
  return String();
}

String relayState(int numRelay){
  if(RELAY_NO){
    if(digitalRead(relayGPIOs[numRelay-1])){
      return "";
    }
    else {
      return "checked";
    }
  }
  else {
    if(digitalRead(relayGPIOs[numRelay-1])){
      return "checked";
    }
    else {
      return "";
    }
  }
  return "";
}

void rootPage() {
   String content = String(index_html); 
   content.replace("%BUTTONPLACEHOLDER%", String(processor1("BUTTONPLACEHOLDER")));
   Server.send(200, "text/html", content);
}
void sendRedirect(String uri) {
  WebServerClass& server = Portal.host();
  server.sendHeader("Location", uri, true);
  server.send(302, "text/plain", "");
  server.client().stop();
}
void updatepage() {
   WebServerClass& server = Portal.host();

    String inputMessage;
    String inputParam;
    String inputMessage2;
      inputMessage = server.arg("relay");
      
      inputMessage2 = server.arg("state");
      
    if (server.arg("relay")!=0 && server.arg("state")!=0) {
      if(RELAY_NO){
        Serial.print("NO ");
        digitalWrite(relayGPIOs[inputMessage.toInt()-1], !inputMessage2.toInt());
      }
      else{
        Serial.print("NC ");
        digitalWrite(relayGPIOs[inputMessage.toInt()-1], inputMessage2.toInt());
      }
    }
    else {
      inputMessage = "No message sent";
      inputParam = "none";
    }
  sendRedirect("/");
}

void setup() {
  delay(1000);
  Serial.begin(115200);
  Serial.println();
  
  for(int i=1; i<=NUM_RELAYS; i++){
    pinMode(relayGPIOs[i-1], OUTPUT);
    if(RELAY_NO){
      digitalWrite(relayGPIOs[i-1], HIGH);
    }
    else{
      digitalWrite(relayGPIOs[i-1], LOW);
    }
  }
  
  // Enable saved past credential by autoReconnect option,
  // even once it is disconnected.
  Config.autoReconnect = true;
  Portal.config(Config);

  // Behavior a root path of ESP8266WebServer.
  Server.on("/", rootPage);
  Server.on("/update",updatepage);
  // Establish a connection with an autoReconnect option.
  if (Portal.begin()) {
    Serial.println("WiFi connected: " + WiFi.localIP().toString());
  }
}

void loop() {
  Portal.handleClient();
}
