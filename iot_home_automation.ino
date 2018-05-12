#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <TimeLib.h>
#include <NtpClientLib.h>
#include <RCSwitch.h>
#include <Ticker.h>
#include <ArduinoJson.h>
#include <ESP8266HTTPClient.h>
#include <TaskScheduler.h>
#include <FS.h>
#include <WiFiUdp.h>

#include "constants.h"

ADC_MODE(ADC_VCC);

const int ioPins[] = { D0, D1, D2, D3, D4, D5, D6, D7, D8 };

WiFiUDP udp;
Ticker ticker;
Scheduler scheduler;
RCSwitch mySwitch = RCSwitch();
ESP8266WebServer server(WEBSERVER_PORT);
const size_t bufferSize = JSON_OBJECT_SIZE(3) + 40;
DynamicJsonBuffer jsonBuffer(bufferSize);

struct _weatherInfo {
  float hom = 0;
  int eso_1ora = 0;
  int eso_ma = 0;
  time_t weather_updated = 0;
} weatherInfo;

Task tWeatherUpdate(WEATHER_UPDATE_MINS * TASK_MINUTE, TASK_FOREVER, &updateWeather, &scheduler, true);


void setPin(int state) {
  digitalWrite(LED_PIN, state);
}


void setupPins() {
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, OFF);
  
  pinMode(RELAY1_PIN, OUTPUT);
  pinMode(RELAY2_PIN, OUTPUT);
  digitalWrite(RELAY1_PIN, OFF);
  digitalWrite(RELAY2_PIN, OFF);

  mySwitch.enableTransmit(RC_PIN);
}

void setupWifiClient() {
  WiFi.mode(WIFI_STA);
  WiFi.hostname(DHCP_CLIENTNAME);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.println("");
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(WIFI_SSID);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void setupRequestHandlers() {
  /** Core stuff **/
  server.serveStatic("/", SPIFFS, "index.htm");
  server.on("/led", HTTP_POST, handleLED);
  server.on("/timed", HTTP_POST, handleTimed);
  server.on("/info", handleInfo);
  server.onNotFound(handleNotFound);

  /** RC Switch **/
  server.on("/lampa-on", HTTP_POST, handleLampaOn);
  server.on("/lampa-off", HTTP_POST, handleLampaOff);
  server.on("/kapu-auto", HTTP_POST, handleKapuAuto);
  server.on("/kapu-gyalog", HTTP_POST, handleKapuGyalog);

  /** Relay **/
  server.on("/relay1-on", HTTP_POST, handleRelay1On);
  server.on("/relay1-off", HTTP_POST, handleRelay1Off);
  server.on("/relay2-on", HTTP_POST, handleRelay2On);
  server.on("/relay2-off", HTTP_POST, handleRelay2Off);

  /** UDP packet **/
  server.on("/wakeup", HTTP_POST, handleWolWakeup);
}

void setup(void){
  Serial.begin(SERIAL_BAUD_RATE);
  
  setupPins();

  setupWifiClient();
   
  MDNS.begin(DHCP_CLIENTNAME);
  SPIFFS.begin();
  NTP.begin();

  setupRequestHandlers();

  server.begin();
  Serial.println("HTTP server started");

  MDNS.addService("http", "tcp", WEBSERVER_PORT);
}

void loop(void){
  server.handleClient();
  scheduler.execute();
}

