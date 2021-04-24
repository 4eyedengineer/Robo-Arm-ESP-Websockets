#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <WebSocketsServer.h>
#include <DNSServer.h>

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
  server.onNotFound(handle_NotFound);
  server.begin();
  Serial.println("HTTP server started");
  Serial.println("Starting WS server");
  startWebSocket();
  dnsServer.start(DNS_PORT, "*", gateway);
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
      break;
  }
}

void handle_OnConnect() {
  Serial.println("Client Request");
  server.send(200, "text/html", F("<!DOCTYPE html><html> <head> <style>div{padding: 20px; border: 2px solid;}.box{margin-left: auto; margin-right: auto; width: 300px; padding: 20px; border: 2px solid gray;}input{width: 100%;}</style> </head> <body> <h1 style=\"margin: auto;width: 200px;\">Control Me</h1> <div class=\"box\"> Control Arm: X axis <input oninput=\"armXChange(this.value)\" type=\"range\" min=\"0\" max=\"100\" value=\"50\" class=\"\" id=\"arm-x\"> </div><div class=\"box\"> Control Arm: Y axis <input oninput=\"armXChange(this.value)\" type=\"range\" min=\"0\" max=\"100\" value=\"50\" class=\"\" id=\"arm-y\"> </div><div class=\"box\"> Control Arm: Z axis <input oninput=\"armXChange(this.value)\" type=\"range\" min=\"0\" max=\"100\" value=\"50\" class=\"\" id=\"arm-z\"> </div><script>var ws; function connWs(){ws=new WebSocket(`ws://${document.location.host}:81`); ws.onmessage=(m=>{console.log(m);})}function armXChange(v){console.log(v); ws.send(`${v}`);}(function x(){connWs()})(); </script> </body></html>"));
}

void handle_NotFound(){
  server.send(404, "text/plain", "Not found");
}


// ----------------------------------------------------------------- //
// --------------------------- Web Pages --------------------------- //
// ----------------------------------------------------------------- //
String makeHtmlPage(String head, String style, String body)
{
  return String(" \
        <!DOCTYPE html> \
        <html> \
        <head> \
        ") + head + String(" \
        <style> \
        ") + style + String(" \
        </head> \
        </style> \
        <body> \
        ") + body + String(" \
        </body> \
        </html> \
    ");    
}

String SendHTML(String text){
  Serial.println("Sending HTML");
  String ptr = "<!DOCTYPE html> <html>\n";
  ptr +="<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  ptr +="<title>LED Control</title>\n";
  ptr +="</head>\n";
  ptr +="<body>\n";
  ptr +="<h1>ESP8266 Web Server</h1>\n";
  ptr +=text + "\n";
  ptr +="<h3>Using Access Point(AP) Mode</h3>\n";
  ptr +="</body>\n";
  ptr +="</html>\n";
  return ptr;
}
