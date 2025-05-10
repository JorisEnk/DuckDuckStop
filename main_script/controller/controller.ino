#include <WiFi.h>
#include <time.h>

// WLAN
const char* ssid = "TP-Link_F574";
const char* password = "38433948";

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

void loop() {
  float price = getCurrentPrice();
  float radiation = getCurrentRadiation();
  float grid = getForecastLoad();
  float wind = getAverageWindNext8Hours();

  Serial.printf("\n Wind: %.4f , Preis: %.3f €/MWh, ☀️ Strahlung: %.2f W/m², Grid: %.1f \n", wind, price, radiation, grid);

  bool shouldTurnOn = price <= priceLimit && radiation >= radiationLimit;

  // Beide Geräte steuern
  toggleShelly(shouldTurnOn);  // Steckdose schalten
  toggleShellyBulb(shouldTurnOn);  // Glühbirne schalten

  delay(1 * 60 * 1000); // alle 30 Minuten wiederholen
}
