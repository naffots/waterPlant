#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

extern "C" {
#include "user_interface.h"
}

const char* ssid = "aaaaaaaaaa";
const char* password = "bbbbbbbb";
int led = 2;
int pumpPin = 5;
int humidityVccPin = 4;
int humiditySensorPin = 0;
double humidity = 0;
bool readHumiditySwitch = true;

// Timer
os_timer_t myTimer;
#define HUMIDITY_SPEED 30000

ESP8266WebServer server(80);

void handleHumidity()
{
  server.send(200, "text/html", String(humidity));
}

void handleRoot() {
  server.send(200, "text/html", "<h1>Water plant server</h1>");
}

void humidityCallback(void *pArg) {
  readHumiditySwitch = true;
} 

void readHumidity()
{
  int n = 5;
  double sum = 0;
  double average;
  
  digitalWrite(led, LOW);
  digitalWrite(humidityVccPin, HIGH);
  delay(500);

  for (int i = 0; i < n; i++)
  {
    sum = sum + analogRead(humiditySensorPin);
  }
  
  humidity = sum / n;
  
  digitalWrite(humidityVccPin, LOW);
  digitalWrite(led, HIGH);
}

void handleReset() {
  server.send(200, "text/html", "Rebooting!");
  ESP.reset();
}

void handleNotFound(){
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i<server.args(); i++){
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

void setup(void){
  pinMode(led, OUTPUT);  
  pinMode(pumpPin, OUTPUT);
  pinMode(humidityVccPin, OUTPUT);
  digitalWrite(led, LOW);

  Serial.begin(115200);
  Serial.println("Connecting");
  WiFi.begin(ssid, password);
  WiFi.mode(WIFI_STA);
  Serial.println("Started");

  // WIFI
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // WebServer
  server.on("/", handleRoot);
  server.on("/humidity", handleHumidity);
  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println("HTTP server started");
  
  // Timer
  os_timer_setfn(&myTimer, humidityCallback, NULL);
  os_timer_arm(&myTimer, HUMIDITY_SPEED, true);
  digitalWrite(led, HIGH);
}

void loop(void){
  server.handleClient();

  if (readHumiditySwitch)
  {
    readHumidity();
    readHumiditySwitch = false;
  }  

  delay(1);
}
