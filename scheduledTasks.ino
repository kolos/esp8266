void uploadTemperature() {
  tempSensors.requestTemperatures();

  String message = "";
  for(int i=0; i < NUM_OF_TEMP_SENSORS;i++) {
    if(tempSensors.getAddress(tempDeviceAddress, i)) {
      message += "temp";
      message += i;
      message += "=";
      message += tempSensors.getTempC(tempDeviceAddress);
      message += "&";
    }
  }
  
  HTTPClient http;
  http.begin(TEMP_UPLOAD_URL);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  http.POST(message);
  http.end();
}

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

  pinState = 0;
  pinTouched = 0;
  
  for(muCron timer: timers) {
    if(!timer.enabled) continue;

    TimeElements _target;
    breakTime(now(), _target);
    _target.Hour = timer.start_hour;
    _target.Minute = timer.start_minute;
    time_t target = makeTime(_target);
    
    if((timer.day_of_week & (1 << dayOfWeek(target))) != 0 && now() >= target && now() < target + 60 * timer.operating_minutes) {
      pinState |= 1 << timer.pin; // set PIN STATE ON      
    } else {
      pinState |= 0 << timer.pin; // set PIN STATE OFF
    }
    pinTouched |= 1 << timer.pin; // set PIN TOUCH true
  }

  for(int pin=0;pin<sizeof(pinState) * 8; pin++) {
    if((pinTouched & (1 << pin)) != 0) {
      digitalWrite(pin, ((pinState & (1 << pin)) != 0) ? ON : OFF);
    }
  }
}

