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
    Serial.println("❌ Keine Zeit verfügbar");
    return;
  }
}

void loop() {
  float price = getCurrentPrice();
  float radiation = getCurrentRadiation();

  Serial.printf("\n📈 Preis: %.2f €/MWh, ☀️ Strahlung: %.1f W/m²\n", price, radiation);
  
  if (price <= priceLimit && radiation >= radiationLimit) {
    toggleShelly(true);
  } else {
    toggleShelly(false);
  }

  delay(30 * 60 * 1000); // alle 30 Minuten wiederholen
}
