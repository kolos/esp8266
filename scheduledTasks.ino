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
