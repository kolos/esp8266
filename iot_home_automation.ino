#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266NetBIOS.h>
#include <TimeLib.h>
#include <NtpClientLib.h>
#include <RCSwitch.h>
#include <Ticker.h>
#include <ArduinoJson.h>
#include <ESP8266HTTPClient.h>
#include <TaskScheduler.h>
#include <FS.h>
#include <WiFiUdp.h>
#include <vector>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <WebSocketsServer.h>

#include "constants.h"

ADC_MODE(ADC_VCC);

const byte ioPins[] = { D0, D1, D2, D3, D4, D5, D6, D7, D8 };

WiFiUDP udp;
Ticker ticker;
Scheduler scheduler;
RCSwitch mySwitch = RCSwitch();
ESP8266WebServer server(WEBSERVER_PORT);
const size_t bufferSize = JSON_OBJECT_SIZE(3) + 40;
DynamicJsonBuffer jsonBuffer(bufferSize);
ESP8266WiFiMulti wifiMulti;
OneWire oneWire(ONEWIRE_PIN);
DallasTemperature tempSensors(&oneWire);
DeviceAddress tempDeviceAddress;
File fsUploadFile;
WebSocketsServer webSocket = WebSocketsServer(WEBSOCKET_PORT);

struct _weatherInfo {
  float hom = 0;
  int eso_1ora = 0;
  int eso_ma = 0;
  time_t weather_updated = 0;
} weatherInfo;

struct muCron {
  unsigned int operating_minutes:8;
  unsigned int day_of_week:8;
  unsigned int enabled:1;
  unsigned int pin:4;
  unsigned int start_hour:5;
  unsigned int start_minute:6;
};

int32_t pinState, pinTouched;

vector<muCron> timers;

Task tWeatherUpdate(WEATHER_UPDATE_MINS * TASK_MINUTE, TASK_FOREVER, &updateWeather, &scheduler, true);
Task tUploadTemperature(TEMP_UPLOAD_MINS * TASK_MINUTE, TASK_FOREVER, &uploadTemperature, &scheduler, true);
Task tCronUpdate(TASK_SECOND, TASK_FOREVER, &updateCronSecond, &scheduler, false);


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
  
  tempSensors.begin();
  mySwitch.enableTransmit(RC_TRANSMIT_PIN);
  mySwitch.enableReceive(RC_RECEIVE_PIN);
  
}

void setupWifiClient() {
  WiFi.mode(WIFI_AP_STA);
  WiFi.hostname(DHCP_CLIENTNAME);

  WiFi.softAP(WIFI_AP_SSID, WIFI_AP_PASSWORD);
  wifiMulti.addAP(WIFI_SSID, WIFI_PASSWORD);
  wifiMulti.addAP(WIFI_SSID_2, WIFI_PASSWORD_2);
  Serial.println("");
  Serial.print("Connecting Wifi");
  
  while (wifiMulti.run() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(WiFi.SSID());
  Serial.print("Local IP address: ");
  Serial.println(WiFi.localIP());
  Serial.print("SoftAP IP address: ");
  Serial.println(WiFi.softAPIP());
}

void setupRequestHandlers() {
  /** Core stuff **/
  server.on("/led", HTTP_POST, handleLED);
  server.on("/scheduler", HTTP_POST, handleScheduler);
  server.on("/scheduler_reset", HTTP_POST, handleSchedulerReset);
  server.on("/upload_file", HTTP_POST, []() {
      server.send(200, "text/plain", "");
  }, handleFileUpload);
  server.on("/scheduler_store", HTTP_POST, handleStoreSchedulerConfig);
  server.on("/info", handleInfo);
  server.serveStatic("/", SPIFFS, "/");
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

void setupTimers() {
   
  File configFile = SPIFFS.open("/scheduler_config.bin", "r");
  if(configFile) {
   timers.clear();
   muCron tmp;

   for(int i=0; i<configFile.size(); i+= sizeof(muCron)){
     configFile.readBytes((char*)&tmp, sizeof(muCron));
     timers.push_back(tmp);
   }
  }
  configFile.close();

  Serial.print("Scheduled tasks: ");
  Serial.println(timers.size());
}

void setupNtpSyncEvent() {
  NTP.onNTPSyncEvent ([](NTPSyncEvent_t event) {
    if(event == timeSyncd) {
      tCronUpdate.enable();
    }
  });
}

void setupWebsocket() {
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
}

void rcReceiveHandle() {
  if (mySwitch.available()) {
    int value = mySwitch.getReceivedValue();
    if (value != 0) {
      String msg = "{\"val\":";
      msg += mySwitch.getReceivedValue();
      msg += ", \"len\":";
      msg += mySwitch.getReceivedBitlength();
      msg += ", \"prot\":";
      msg += mySwitch.getReceivedProtocol();
      msg += "}";
      webSocket.broadcastTXT(msg);
    }
  
    mySwitch.resetAvailable();
  }
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
    switch(type) {
        case WStype_DISCONNECTED:
            break;
        case WStype_CONNECTED:
            break;
        case WStype_TEXT:
            break;
        case WStype_BIN:
            break;
    }
}

void setup(void){
  Serial.begin(SERIAL_BAUD_RATE);
  
  setupPins();
  setupWifiClient();
  setupNtpSyncEvent();

  NBNS.begin(DHCP_CLIENTNAME);
  SPIFFS.begin();
  NTP.begin();
  
  setupRequestHandlers();
  setupTimers();
  setupWebsocket();

  server.begin();
  Serial.println("HTTP server started");
}

void loop(void){
  server.handleClient();
  scheduler.execute();
  webSocket.loop();
  rcReceiveHandle();
}

