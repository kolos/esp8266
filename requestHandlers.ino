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

  String message = "{\"time\":";
  message += now();
  message += ",\"pins\":";
  message += pinVal;
  message += ",\"wifi\":";
  message += WiFi.RSSI();
  message += ",\"uptime\":";
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
  message += "}";
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "application/json", message);
}

void handleTimed() {
  unsigned int milis = 0;
  String message = "";

  if (server.arg("seconds")== ""){  
    message = "nincs parameter megadva!";
  }else{
    milis = server.arg("seconds").toInt() * 1000;

    ticker.once_ms(milis, setPin, LOW);
    message = "parameter => ";
    message += milis;
  }
  
  server.send(200, "text/plain", message);
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
