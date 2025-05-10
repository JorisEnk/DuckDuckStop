#include <HTTPClient.h>
#include <ArduinoJson.h>

const char* awattar_url = "https://api.awattar.de/v1/marketdata";

float getCurrentPrice() {
  HTTPClient http;
  http.begin(awattar_url);
  int httpCode = http.GET();

  if (httpCode != 200) {
    Serial.printf("Awattar Fehler: %d\n", httpCode);
    http.end();
    return 9999.0;
  }

  String payload = http.getString();
  DynamicJsonDocument doc(32 * 1024);
  if (deserializeJson(doc, payload)) {
    http.end();
    return 9999.0;
  }

  long now = time(nullptr) * 1000;
  JsonArray data = doc["data"];
  for (JsonObject entry : data) {
    long start = entry["start_timestamp"];
    long end = entry["end_timestamp"];
    if (now >= start && now < end) {
      float price = entry["marketprice"];
      http.end();
      return price;
    }
  }

  http.end();
  return 9999.0;
}
