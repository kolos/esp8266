void handleFileUpload() {
  HTTPUpload& upload = server.upload();

  if(upload.status == UPLOAD_FILE_START){
    String filename = upload.filename;
    if(!filename.startsWith("/")) filename = "/"+filename;
    fsUploadFile = SPIFFS.open(filename, "w");
  } else if(upload.status == UPLOAD_FILE_WRITE){
    if(fsUploadFile) {
      fsUploadFile.write(upload.buf, upload.currentSize);
    }
  } else if(upload.status == UPLOAD_FILE_END){
    if(fsUploadFile) {
      fsUploadFile.close();
      server.sendHeader("Refresh","5; url=/");
      server.send(200, "text/plain", "File uploaded, redirect in 5 secs..");
    } else {
      server.send(500, "text/plain", "500: couldn't create file");
    }
  }
}

void handleStoreSchedulerConfig() {
  File configFile = SPIFFS.open("/scheduler_config.bin", "w");
        
  if (!configFile) {
    server.send(500, "text/plain", "500: couldn't create file");
  } else {
    for(muCron timer: timers) {
      unsigned char * data = reinterpret_cast<unsigned char*>(&timer);
      configFile.write(data, sizeof(muCron));
    }
    configFile.close();
    server.send(200, "text/plain", "SchedulerConfig saved.");
  }
}

void handleLED() {
  int led_state = !digitalRead(LED_PIN);
  digitalWrite(LED_PIN, led_state);
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "text/plain", !led_state ? "LED: ON" : "LED: OFF");
}


void handleInfo() {
  long pinVal = 0;
  for(int i=0; i< sizeof ioPins; i++) {
    pinVal ^= digitalRead(ioPins[i]) << ioPins[i];
  }

  tempSensors.requestTemperatures();

  String message = "{\"time\":";
  message += now();
  message += ",\"pins\":";
  message += pinVal;
  message += ",\"wifi\":";
  message += WiFi.RSSI();
  message += ",\"wifi_ssid\": \"";
  message += WiFi.SSID();
  message += "\",\"uptime\":";
  message += NTP.getUptime();
  message += ",\"hom\":";
  message += weatherInfo.hom;
  message += ",\"eso_1ora\":";
  message += weatherInfo.eso_1ora;
  message += ",\"eso_ma\":";
  message += weatherInfo.eso_ma;
  message += ",\"weather_updated\":";
  message += weatherInfo.weather_updated;
  message += ",\"vcc\":";
  message += ESP.getVcc();
  message += ",\"heap\":";
  message += ESP.getFreeHeap();
  message += ",\"lights\": [";
  for(byte i = 0; i<LIGHT_DETECT_HOURS; i++) {
   message += lightsDetectedArr[i];
   message += ","; 
  }
  message += "null]";
  message += ",\"lights2\": [";
  for(byte i = 0; i<LIGHT_DETECT_HOURS; i++) {
   message += lightsDetectedArr2[i];
   message += ","; 
  }
  message += "null]";
  message += ",\"timers\": [";
  for(muCron timer: timers) {
    message += "{";
    message += "\"day_of_week\": ";
    message += timer.day_of_week;
    message += ",\"start_hour\": ";
    message += timer.start_hour;
    message += ",\"start_minute\": ";
    message += timer.start_minute;
    message += ",\"operating_minutes\": ";
    message += timer.operating_minutes;
    message += ",\"pin\": ";
    message += timer.pin;
    message += ",\"enabled\": ";
    message += timer.enabled;
    message += "}, ";
  }  
  message += "null]";
  message += ",\"temp\": [";
  for(int i=0; i < NUM_OF_TEMP_SENSORS;i++) {    
    if(tempSensors.getAddress(tempDeviceAddress, i)) {
      message += tempSensors.getTempC(tempDeviceAddress);
      message += ",";
    }
  }
  message += "null]";
  message += "}";
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "application/json", message);
}

void handleScheduler() {

  if(server.args() == 5
    && server.hasArg("day_of_week")
    && server.hasArg("start_hour")
    && server.hasArg("start_minute")
    && server.hasArg("operating_minutes")
    && server.hasArg("pin")
  ) {
    muCron _tmp;
    _tmp.day_of_week = server.arg("day_of_week").toInt();
    _tmp.start_hour = server.arg("start_hour").toInt();
    _tmp.start_minute = server.arg("start_minute").toInt();
    _tmp.operating_minutes = server.arg("operating_minutes").toInt();
    _tmp.pin = server.arg("pin").toInt();
    _tmp.enabled = 1;
    timers.push_back(_tmp);

    server.send(200, "text/plain", "ok");
  } else {
    server.send(400, "text/plain", "need params: day_of_week, start_hour, start_minute, operation_minutes, pin");
  }
}

void handleSchedulerReset() {
  timers.clear();
  server.send(200, "text/plain", "ok");
}

void handleLampaOn() {
  mySwitch.setProtocol(RC_LAMPA_PROTOCOL); // lámpa
  mySwitch.send(RC_LAMPA_EVENT_ON, RC_LAMPA_EVENT_LENGTH);
  server.send(200, "text/plain", "lampa-on");
}

void handleLampaOff() {
  mySwitch.setProtocol(RC_LAMPA_PROTOCOL); // lámpa
  mySwitch.send(RC_LAMPA_EVENT_OFF, RC_LAMPA_EVENT_LENGTH);
  server.send(200, "text/plain", "lampa-off");
}

void handleKapuAuto() {
  mySwitch.setProtocol(RC_KAPU_PROTOCOL); // kapu
  mySwitch.send(RC_KAPU_EVENT_AUTO, RC_KAPU_EVENT_LENGTH);
  server.send(200, "text/plain", "kapu-auto");
}

void handleKapuGyalog() {
  mySwitch.setProtocol(RC_KAPU_PROTOCOL); // kapu
  mySwitch.send(RC_KAPU_EVENT_GYALOG, RC_KAPU_EVENT_LENGTH);
  server.send(200, "text/plain", "kapu-gyalog");
}

void handleRelay1On() {
  digitalWrite(RELAY1_PIN, ON);
  server.send(200, "text/plain", "relay1-on");
}

void handleRelay1Off() {
  digitalWrite(RELAY1_PIN, OFF);
  server.send(200, "text/plain", "relay1-off");
}

void handleRelay2On() {
  digitalWrite(RELAY2_PIN, ON);
  server.send(200, "text/plain", "relay2-on");
}

void handleRelay2Off() {
  digitalWrite(RELAY2_PIN, OFF);
  server.send(200, "text/plain", "relay2-off");
}

void handleWolWakeup() {
  byte targetAddr[6] = WOL_TARGET_MAC;
  byte bcastAddr[4] = {0xFF, 0xFF, 0xFF, 0xFF};
  byte magicPacket[102] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
  for(byte i=1;i<=16;i++) {
    memcpy(magicPacket + i*6 , targetAddr, sizeof targetAddr);
  }
  
  udp.beginPacket(bcastAddr, WOL_UDP_PORT);
  udp.write(magicPacket, sizeof magicPacket);
  udp.endPacket();
  server.send(200, "text/plain", "wakeup");
}

void handleNotFound() {
  server.send(404, "text/plain", "404: Not Found");
}
