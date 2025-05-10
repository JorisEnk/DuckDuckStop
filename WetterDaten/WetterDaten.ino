#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <time.h>

// WLAN-Zugang
const char* ssid = "TP-Link_F574";
const char* password = "38433948";

// Standort Freiburg
const char* latitude = "47.9990";
const char* longitude = "7.8421";

// NTP-Zeit
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 3600;
const int daylightOffset_sec = 3600;

// Nächste 12 Stunden
const int forecast_hours = 12;

void setup() {
  Serial.begin(115200);
  delay(1000);

  // WLAN verbinden
  WiFi.begin(ssid, password);
  Serial.print("Verbinde mit WLAN");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n✅ WLAN verbunden.");

  // Zeit synchronisieren
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("❌ Keine Zeit verfügbar.");
    return;
  }

  char date_today[11];
  char date_tomorrow[11];
  strftime(date_today, sizeof(date_today), "%Y-%m-%d", &timeinfo);
  time_t tomorrow_ts = mktime(&timeinfo) + 86400;
  struct tm* tomorrow_tm = localtime(&tomorrow_ts);
  strftime(date_tomorrow, sizeof(date_tomorrow), "%Y-%m-%d", tomorrow_tm);

  // API-URL
  String url = "https://api.open-meteo.com/v1/forecast?";
  url += "latitude=" + String(latitude);
  url += "&longitude=" + String(longitude);
  url += "&hourly=temperature_2m,cloudcover,direct_radiation,diffuse_radiation";
  url += "&start_date=" + String(date_today);
  url += "&end_date=" + String(date_tomorrow);
  url += "&timezone=Europe%2FBerlin";

  Serial.println("⏳ Wetterdaten abrufen...");
  HTTPClient http;
  http.begin(url);
  int httpCode = http.GET();

  if (httpCode == 200) {
    String payload = http.getString();
    DynamicJsonDocument doc(32 * 1024);
    DeserializationError error = deserializeJson(doc, payload);

    if (error) {
      Serial.println("❌ JSON-Fehler: ");
      Serial.println(error.f_str());
      return;
    }

    JsonObject hourly = doc["hourly"];
    JsonArray timeArray = hourly["time"];
    JsonArray directArray = hourly["direct_radiation"];
    JsonArray tempArray = hourly["temperature_2m"];
    JsonArray cloudArray = hourly["cloudcover"];
    JsonArray diffuseArray = hourly["diffuse_radiation"];

    time_t now_ts = time(nullptr);
    struct tm* now_tm = localtime(&now_ts);
    char now_str[20];
    strftime(now_str, sizeof(now_str), "%Y-%m-%dT%H:00", now_tm);
    String currentTimeISO = String(now_str);

    Serial.println("\n🌤 Wetterdaten für die nächsten 12 Stunden (Freiburg):\n");

    int count = 0;
    for (int i = 0; i < timeArray.size(); i++) {
      String timeStr = timeArray[i];

      if (timeStr < currentTimeISO) continue;
      if (count >= forecast_hours) break;

      float direct = directArray[i];
      float temp = tempArray[i];
      int cloud = cloudArray[i];
      float diffuse = diffuseArray[i];

      // Analyse
      bool sunlight = false;
      String strength, action;

      if (direct == 0) {
        strength = "none";
        action = "idle";
      } else if (direct < 200) {
        strength = "low";
        action = "store";
        sunlight = true;
      } else if (direct < 500) {
        strength = "medium";
        action = "store";
        sunlight = true;
      } else {
        strength = "high";
        action = "feed_in";
        sunlight = true;
      }

      // Ausgabe
      Serial.printf(
        "%s\n  ↳ direct: %.1f W/m², diffuse: %.1f W/m²\n  ↳ temp: %.1f°C, cloud: %d%%\n  ↳ sunlight: %s, strength: %s, action: %s\n\n",
        timeStr.c_str(),
        direct,
        diffuse,
        temp,
        cloud,
        sunlight ? "yes" : "no",
        strength.c_str(),
        action.c_str()
      );

      count++;
    }

  } else {
    Serial.printf("❌ HTTP Fehler: %d\n", httpCode);
  }

  http.end();
}

void loop() {
  // einmalige Analyse in setup()
}
