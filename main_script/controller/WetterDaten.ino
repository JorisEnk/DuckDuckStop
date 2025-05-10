#include <HTTPClient.h>
#include <ArduinoJson.h>

const char* latitude = "47.9990";
const char* longitude = "7.8421";

float getCurrentRadiation() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) return 0;

  char date_today[11], date_tomorrow[11], now_iso[20];
  strftime(date_today, sizeof(date_today), "%Y-%m-%d", &timeinfo);
  time_t tomorrow_ts = mktime(&timeinfo) + 86400;
  struct tm* tomorrow_tm = localtime(&tomorrow_ts);
  strftime(date_tomorrow, sizeof(date_tomorrow), "%Y-%m-%d", tomorrow_tm);
  strftime(now_iso, sizeof(now_iso), "%Y-%m-%dT%H:00", &timeinfo);

  String url = "https://api.open-meteo.com/v1/forecast?";
  url += "latitude=" + String(latitude);
  url += "&longitude=" + String(longitude);
  url += "&hourly=direct_radiation";
  url += "&start_date=" + String(date_today);
  url += "&end_date=" + String(date_tomorrow);
  url += "&timezone=Europe%2FBerlin";

  HTTPClient http;
  http.begin(url);
  int httpCode = http.GET();

  if (httpCode != 200) {
    Serial.printf("Wetter HTTP Fehler: %d\n", httpCode);
    http.end();
    return 0.0;
  }

  String payload = http.getString();
  DynamicJsonDocument doc(32 * 1024);
  if (deserializeJson(doc, payload)) {
    http.end();
    return 0.0;
  }

  JsonArray timeArray = doc["hourly"]["time"];
  JsonArray radiationArray = doc["hourly"]["direct_radiation"];

  for (int i = 0; i < timeArray.size(); i++) {
    if (timeArray[i].as<String>() == now_iso) {
      float radiation = radiationArray[i];
      http.end();
      return radiation;
    }
  }

  http.end();
  return 0.0;
}
