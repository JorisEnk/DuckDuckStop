#include <WiFi.h>
#include <HTTPClient.h>

extern String shellyBulbSSID;
extern String shellyBulbPASS;
extern String homeSSID;
extern String homePASS;
extern String shellyBulbIP;


void connectToShellyBulbWiFi() {
  Serial.println("🔄 Verbinde mit ShellyBulb-WLAN...");
  Serial.println("📶 SSID: " + shellyBulbSSID);
  Serial.println("🔑 PASS: " + shellyBulbPASS);

  WiFi.disconnect(true);
  delay(500);
  WiFi.begin(shellyBulbSSID.c_str(), shellyBulbPASS.c_str());

  for (int i = 0; i < 20 && WiFi.status() != WL_CONNECTED; i++) {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n✅ Verbunden mit ShellyBulb");
  } else {
    Serial.println("\n❌ Verbindung zur ShellyBulb fehlgeschlagen");
  }
}


void connectToHomeWiFi() {
  Serial.println("🔄 Verbinde mit Heim-WLAN...");
  Serial.println("📶 SSID: " + homeSSID);
  Serial.println("🔑 PASS: " + homePASS);

  Serial.println("🔄 Verbinde zurück zum Heimnetz...");
  WiFi.disconnect(true);
  delay(500);
  WiFi.begin(homeSSID.c_str(), homePASS.c_str());

  for (int i = 0; i < 20 && WiFi.status() != WL_CONNECTED; i++) {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n✅ Zurück im Heimnetz");
  } else {
    Serial.println("\n❌ Heimnetzverbindung fehlgeschlagen");
  }
}

void setShellyBulbColor(int r, int g, int b) {
  connectToShellyBulbWiFi();

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("❌ Keine WLAN-Verbindung zur ShellyBulb");
    connectToHomeWiFi();
    return;
  }

  HTTPClient http;

  // Setze RGB-Modus + Farbe + Helligkeit (100 %)
  String url = String("http://") + shellyBulbIP +
               "/light/0?turn=on&mode=color&red=" + String(r) +
               "&green=" + String(g) +
               "&blue=" + String(b) +
               "&gain=100";  // oder: "&brightness=100" je nach Firmware

  Serial.println("📡 Sende an ShellyBulb: " + url);
  http.begin(url);
  int httpCode = http.GET();

  if (httpCode > 0) {
    Serial.printf("💡 RGB gesetzt: R=%d G=%d B=%d (HTTP %d)\n", r, g, b, httpCode);
  } else {
    Serial.printf("❌ Fehler beim Setzen der Farbe: %s\n", http.errorToString(httpCode).c_str());
  }

  http.end();

  connectToHomeWiFi();
}


// Glühbirne schalten
void toggleShellyBulb(bool state) {
  connectToShellyBulbWiFi();

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("❌ Keine WLAN-Verbindung zur ShellyBulb");
    return;
  }

  HTTPClient http;
  String url = String("http://") + shellyBulbIP + "/light/0?turn=" + (state ? "on" : "off");

  http.begin(url);
  int httpCode = http.GET();

  if (httpCode > 0) {
    Serial.printf("💡 ShellyBulb geschaltet: %s (HTTP %d)\n", state ? "an" : "aus", httpCode);
  } else {
    Serial.printf("❌ Fehler beim Schalten: %s\n", http.errorToString(httpCode).c_str());
  }

  http.end();

  connectToHomeWiFi();  // wieder zurück ins Heimnetz
}