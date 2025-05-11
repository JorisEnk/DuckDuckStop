// Get the current grid data from Stromgedacht API
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <time.h>

float getForecastLoad() {
  // URL 
  time_t now = time(nullptr);
  struct tm* timeinfo = gmtime(&now);

  char from_buf[11], to_buf[11];
  strftime(from_buf, sizeof(from_buf), "%Y-%m-%d", timeinfo);

  timeinfo->tm_mday += 1;
  mktime(timeinfo);  // normalize the time structure
  strftime(to_buf, sizeof(to_buf), "%Y-%m-%d", timeinfo);

  String url = String("https://api.stromgedacht.de/v1/forecast?zip=79100&from=") + from_buf + "&to=" + to_buf;

  HTTPClient http;
  http.begin(url);
  int httpCode = http.GET();

  if (httpCode != 200) {
    Serial.printf("HTTP error: %d\n", httpCode);
    http.end();
    return -1.0;
  }

  String payload = http.getString();
  http.end();

  const size_t capacity = 64 * 1024;
  DynamicJsonDocument doc(capacity);
  DeserializationError error = deserializeJson(doc, payload);

  if (error) {
    Serial.print("JSON error: ");
    Serial.println(error.f_str());
    return -1.0;
  }

  JsonArray loadArray = doc["load"];
  now = time(nullptr);  // current time in seconds since epoch
  time_t future_limit = now + 3 * 3600;  // +3 hours

  float sum = 0.0;
  int count = 0;

  for (JsonObject entry : loadArray) {
    const char* dt_str = entry["dateTime"];
    float value = entry["value"];

    struct tm dt_tm = {};
    strptime(dt_str, "%Y-%m-%dT%H:%M:%S", &dt_tm);
    time_t dt_time = mktime(&dt_tm);

    // Z no t in UTC
    if (dt_time >= now && dt_time <= future_limit) {
      sum += value;
      count++;
    }
  }

  if (count == 0) {
    Serial.println("No data available for the next 3 hours");
    return -1.0;
  }

  float avg = sum / count;
  Serial.printf("avg grid (next 3 h): %.2f MW\n", avg);

  if (avg > 5500.0) {
    Serial.println("grid: high");
  } else {
    Serial.println("grid: not high");
  }

  return avg;
}