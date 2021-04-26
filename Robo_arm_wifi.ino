#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <WebSocketsServer.h>
#include <DNSServer.h>
#include <Servo.h>

StaticJsonDocument<64> servoStates;

/* Put your SSID & Password */
const char *ssid = "Control Me Daddy";
const char *password = "hellomyson";

/* Put IP Address details */
IPAddress local_ip(192,168,1,1);
IPAddress gateway(192,168,1,1);
IPAddress subnet(255,255,255,0);
IPAddress DNSip(8,8,4,4);
const byte DNS_PORT = 53;

WebSocketsServer webSocket(81);
ESP8266WebServer server(80);
DNSServer dnsServer;

Servo myservo; // need mutliple


void setup()
{
  // Serial.begin(9600);
  delay(1000);
  Serial.begin(115200);
  Serial.println();
  Serial.print("Configuring access point...");
  WiFi.softAP(ssid, password);
  WiFi.softAPConfig(local_ip, gateway, subnet);
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);
  delay(100);
  server.on("/", handle_OnConnect);
  server.on("/positions", handle_getPositions);
  server.onNotFound(handle_NotFound);
  server.begin();
  Serial.println("HTTP server started");
  
  Serial.println("Starting WS server");
  startWebSocket();
  
  Serial.println("Starting DNS server");
  dnsServer.start(DNS_PORT, "*", gateway);
  Serial.println("DNS server started");


  // setup servos
  delay(100);
  myservo.attach(15);
}

void loop()
{
  dnsServer.processNextRequest();
  server.handleClient();
  webSocket.loop();
}

void startWebSocket() { // Start a WebSocket server
  webSocket.begin();                          // start the websocket server
  webSocket.onEvent(webSocketEvent);          // if there's an incomming websocket message, go to function 'webSocketEvent'
  Serial.println("WebSocket server started.");
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t lenght) { // When a WebSocket message is received
  switch (type) {
    case WStype_DISCONNECTED:             // if the websocket is disconnected
      Serial.printf("[%u] Disconnected!\n", num);
      break;
    case WStype_CONNECTED: {              // if a new websocket connection is established
        IPAddress ip = webSocket.remoteIP(num);
        Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);
      }
      break;
    case WStype_TEXT:                     // if new text data is received
      Serial.printf("[%u] get Text: %s\n", num, payload);
      jsonParse(servoStates, (char*)payload);
      myservo.write(convertToInt(payload));
      break;
  }
}

int convertToInt(uint8_t * payload){
  String str="";
  char c;
  for(int i=0;i<4;i++)
      if (isDigit(c=(char)(*(payload+i))))
          str += c;
  Serial.println("converted string: " + str +"\n");
  int Val =str.toInt();
  Serial.printf("converted int: %d\n", Val);
  return Val>1023?1023:Val;
}

void handle_OnConnect() {
  Serial.println("Client Request");
  server.send(200, "text/html", F("<!DOCTYPE html><html> <head> <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\"> <style>div{padding: 20px; border: 2px solid;}.box{margin-left: auto; margin-right: auto; width: 300px; padding: 20px; border: 2px solid gray;}input{width: 100%;}input[type='range']::-webkit-slider-thumb{width: 25px; -webkit-appearance: none; height: 25px; cursor: ew-resize; background: #434343;}</style> </head> <body> <h1 style=\"margin: auto;width: 200px;\">Control Me</h1> <div class=\"box\"> Control Arm: X axis <input oninput=\"armChange(this.value, 'x')\" type=\"range\" min=\"0\" max=\"180\" value=\"90\" class=\"\" id=\"arm-x\"> </div><div class=\"box\"> Control Arm: Y axis <input oninput=\"armChange(this.value, 'y')\" type=\"range\" min=\"0\" max=\"180\" value=\"90\" class=\"\" id=\"arm-y\"> </div><div class=\"box\"> Control Arm: Z axis <input oninput=\"armChange(this.value, 'z')\" type=\"range\" min=\"0\" max=\"180\" value=\"90\" class=\"\" id=\"arm-z\"> </div><script>var ws; function connWs(){ws=new WebSocket(`ws://${document.location.host}:81`); ws.onmessage=(m=>{console.log(m);})}function armChange(v, axis){const obj={}; obj[axis]=v; ws.send(JSON.stringify(obj));}(function x(){connWs()})(); </script> </body></html>"));
}

void handle_getPositions() {
  Serial.println("sending servo states");
  server.send(200, "text/html", jsonStringify(servoStates));
}

void handle_NotFound(){
  server.send(404, "text/plain", "Not found");
}


// ----------------------------------------------------------------- //
// --------------------------- Web Pages --------------------------- //
// ----------------------------------------------------------------- //
//String makeHtmlPage(String head, String style, String body)
//{
//  return String(" \
//        <!DOCTYPE html> \
//        <html> \
//        <head> \
//        ") + head + String(" \
//        <style> \
//        ") + style + String(" \
//        </head> \
//        </style> \
//        <body> \
//        ") + body + String(" \
//        </body> \
//        </html> \
//    ");    
//}

//String SendHTML(String text){
//  Serial.println("Sending HTML");
//  String ptr = "<!DOCTYPE html> <html>\n";
//  ptr +="<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
//  ptr +="<title>LED Control</title>\n";
//  ptr +="</head>\n";
//  ptr +="<body>\n";
//  ptr +="<h1>ESP8266 Web Server</h1>\n";
//  ptr +=text + "\n";
//  ptr +="<h3>Using Access Point(AP) Mode</h3>\n";
//  ptr +="</body>\n";
//  ptr +="</html>\n";
//  return ptr;
//}


// JSON helper functions
void jsonParse(JsonDocument& doc, char* json) {
  DeserializationError error = deserializeJson(doc, json);
  
  // Test if parsing succeeds.
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
  }
}

char* jsonStringify(JsonDocument& doc) {
//  String jsonString;
char jsonString[64];
  serializeJson(doc, jsonString);
  Serial.printf("stringified obj: %c\n", jsonString);
//  Serial.println("stringified obj: " + jsonString);
  return jsonString;
}
