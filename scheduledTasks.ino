void updateWeather() {
  HTTPClient http;
  http.begin(WEATHER_URL);
  
  int httpCode = http.GET();  
  if (httpCode == HTTP_CODE_OK) {
    String json = http.getString();
    JsonObject& root = jsonBuffer.parseObject(json);

    weatherInfo.hom = root["hom"];
    weatherInfo.eso_1ora = root["eso_1ora"];
    weatherInfo.eso_ma = root["eso_ma"];
    weatherInfo.weather_updated = now();
  }
  
  http.end();
}

void updateCronSecond() {
  if(! NTP.getFirstSync ()) {
    Serial.println("Waiting for first sync..");
    return;
  }

  for(muCron timer: timers) {
    if(!timer.enabled) continue;

    TimeElements _target;
    breakTime(now(), _target);
    _target.Hour = timer.start_hour;
    _target.Minute = timer.start_minute;
    time_t target = makeTime(_target);
    
    if((timer.day_of_week & (1 << dayOfWeek(target))) != 0 && now() >= target && now() < target + 60 * timer.operating_minutes) {
      setPin(ON);
    } else {
      setPin(OFF);
    }
  }
}

