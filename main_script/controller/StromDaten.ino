#include <HTTPClient.h>
#include <ArduinoJson.h>

const char* awattar_url = "https://api.awattar.de/v1/marketdata";

float getCurrentPrice() {
  HTTPClient http;
  http.begin(awattar_url);  // HTTPS geht mit ESP32 (ab ESP32-Arduino v2.x)
  int httpCode = http.GET();

  if (httpCode == 200) {
    String payload = http.getString();

    const size_t capacity = 32 * 1024; // ausreichend für viele Einträge
    DynamicJsonDocument doc(capacity);

    DeserializationError error = deserializeJson(doc, payload);
    if (error) {
      Serial.print("JSON Fehler: ");
      Serial.println(error.f_str());
      return 9999.0;
    }

    JsonArray data = doc["data"];
    Serial.println("\n📈 Strompreise von Awattar (€/MWh):\n");

    for (JsonObject entry : data) {
      long start_ms = entry["start_timestamp"];
      long end_ms = entry["end_timestamp"];
      float price = entry["marketprice"];

      time_t start_sec = start_ms / 1000;
      time_t end_sec = end_ms / 1000;

      struct tm* start_tm = localtime(&start_sec);
      struct tm* end_tm = localtime(&end_sec);

      char start_buf[20], end_buf[6];
      strftime(start_buf, sizeof(start_buf), "%Y-%m-%d %H:%M", start_tm);
      strftime(end_buf, sizeof(end_buf), "%H:%M", end_tm);

      Serial.printf("%s - %s : %.2f €/MWh\n", start_buf, end_buf, price);
      return price;
    }

  } else {
    Serial.printf("HTTP Fehler: %d\n", httpCode);
    return 9999.0;
  }

  http.end();
  return 9999.0;
}
