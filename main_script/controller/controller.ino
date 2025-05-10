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
void berechneRGB(float radiation, float wind, float grid, int& r, int& g, int& b) {
  // Normierung
  float sonne_norm = constrain(radiation / 1000.0, 0.0, 1.0);
  float wind_norm = (wind >= 10) ? constrain((wind - 10.0) / 80.0, 0.0, 1.0) : 0.0;
  float netzlast_norm = constrain((grid - 3000.0) / 4000.0, 0.0, 1.0);
  float netzlast_inv = 1.0 - netzlast_norm;

  // Bewertung
  float erneuerbare = 0.5 * sonne_norm + 0.5 * wind_norm;
  float status = erneuerbare * 0.7 + netzlast_inv * 0.3;

  // Farbverlauf berechnen
  if (status < 0.5) {
    r = 255;
    g = int(255 * (status / 0.5));
    b = 0;
  } else {
    r = int(255 * (1 - (status - 0.5) / 0.5));
    g = 255;
    b = 0;
  }
}


void loop() {
  float price = getCurrentPrice();
  float radiation = getCurrentRadiation();
  float grid = getForecastLoad();
  float wind = getAverageWindNext8Hours();

  Serial.printf("\n🌬️ Wind: %.2f km/h, ☀️ Strahlung: %.2f W/m², ⚡ Netzlast: %.2f MW\n", wind, radiation, grid);

  int r, g, b;
  berechneRGB(radiation, wind, grid, r, g, b);
  setShellyBulbColor(r, g, b);

  delay(1 * 60 * 1000);  // alle 30 Minuten wiederholen
}

