#include <WiFi.h>
#include <time.h>

// WLAN
const char* ssid = "Häck2025";
const char* password = "B#campus!";

// NTP
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 3600;
const int daylightOffset_sec = 3600;

// Grenzwerte
const float priceLimit = 150.0;        // €/MWh
const float radiationLimit = 200.0;    // W/m²

// Externe Funktionen
extern float getCurrentPrice();
extern float getCurrentRadiation();
extern void toggleShelly(bool state);
extern void toggleShellyBulb(bool state);
extern float getForecastLoad();
extern float getAverageWindNext8Hours();

void setup() {
  Serial.begin(115200);
  delay(1000);

  // WLAN verbinden
  WiFi.begin(ssid, password);
  Serial.print("🔌 Verbinde mit WLAN");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n✅ WLAN verbunden.");

  // Zeit synchronisieren
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("❌ Keine Zeit verfügbar");
    return;
  }
  Serial.println("🕒 Zeit synchronisiert");
}

// Hilfsfunktion zum Beschränken eines Werts zwischen min und max
float constrainFloat(float value, float min, float max) {
  if (value < min) return min;
  if (value > max) return max;
  return value;
}

void berechneRGB(float radiation, float wind, float grid, int* r, int* g, int* b) {
  int gridBool = (grid > 5000) ? 1 : 0;
  int radiationBool = (radiation > 250) ? 1 : 0;
  int windBool = (wind > 10 && wind < 90) ? 1 : 0;
  int erneuerbarBool = radiationBool || windBool;

  if (!erneuerbarBool) {
      *r = 255; *g = 0; *b = 0;
      return;
  }

  float radQual = constrainFloat(radiation / 1000.0, 0.0, 1.0);
  float windQual = constrainFloat((wind - 10.0) / 80.0, 0.0, 1.0);
  float qualitaet = 0.5 * radQual + 0.5 * windQual;

  if (gridBool) {
      *r = (int)(50 * (1 - qualitaet));
      *g = (int)(180 + 75 * qualitaet);
      *b = 0;
  } else {
      *r = 255;
      *g = (int)(230 - 80 * (1 - qualitaet));
      *b = 0;
  }
}


void loop() {
  float price = getCurrentPrice();
  float radiation = getCurrentRadiation();
  float grid = getForecastLoad();
  float wind = getAverageWindNext8Hours();

  Serial.printf("\n🌬️ Wind: %.2f km/h, ☀️ Strahlung: %.2f W/m², ⚡ Netzlast: %.2f MW\n", wind, radiation, grid);

  int r, g, b;
  berechneRGB(radiation, wind, grid, &r, &g, &b);
  setShellyBulbColor(r, g, b);

  delay(1 * 60 * 1000);  // alle 30 Minuten wiederholen
}

