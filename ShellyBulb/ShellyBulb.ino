#include <WiFi.h>
#include <HTTPClient.h>
#include <time.h>

// WLAN-Daten
const char* ssid = "shellycolorbulb-485519FC00C9";
const char* password = "";  // leer lassen, falls Shelly ohne Passwort läuft

// Shelly IP
const char* shelly_ip = "192.168.33.1";

unsigned long lastToggleTime = 0;
bool isOn = false;

void setup() {
  Serial.begin(115200);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  Serial.print("Verbinde mit Shelly-WLAN: ");
  Serial.println(ssid);

  // Warte auf Verbindung
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nVerbunden mit Shelly!");
  Serial.print("IP Adresse: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  unsigned long currentTime = millis();

  if (currentTime - lastToggleTime >= 15000) { // alle 15 Sekunden
    toggleBulb();
    lastToggleTime = currentTime;
  }
}

void toggleBulb() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String url = String("http://") + shelly_ip + "/light/0?turn=" + (isOn ? "off" : "on");

    http.begin(url);
    int httpCode = http.GET();

    if (httpCode > 0) {
      Serial.printf("HTTP %d: Lampe %s\n", httpCode, isOn ? "aus" : "an");
      isOn = !isOn;
    } else {
      Serial.printf("Fehler: %s\n", http.errorToString(httpCode).c_str());
    }

    http.end();
  } else {
    Serial.println("Nicht mit WLAN verbunden!");
  }
}
