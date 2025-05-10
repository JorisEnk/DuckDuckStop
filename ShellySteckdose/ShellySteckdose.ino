#include <WiFi.h>
#include <HTTPClient.h>

// WLAN-Zugangsdaten
const char* ssid = "TP-Link_F574";
const char* password = "38433948";

// IP-Adresse der Shelly Steckdose
const char* shelly_ip = "192.168.1.102";

// Zustand speichern
bool isOn = false;
unsigned long lastToggle = 0;
const unsigned long interval = 30 * 1000; // 30 Sekunden

void setup() {
  Serial.begin(115200);
  delay(1000);

  // WLAN verbinden
  WiFi.begin(ssid, password);
  Serial.printf("Verbinde mit WLAN: %s\n", ssid);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\n✅ WLAN verbunden!");
  Serial.print("IP-Adresse: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  unsigned long currentMillis = millis();

  if (currentMillis - lastToggle >= interval) {
    lastToggle = currentMillis;
    toggleShellyPlug();
  }

  delay(100); // Leichtes Delay zur Entlastung
}

void toggleShellyPlug() {
  HTTPClient http;
  String action = isOn ? "off" : "on";
  String url = String("http://") + shelly_ip + "/relay/0?turn=" + action;

  Serial.print("Sende: ");
  Serial.println(url);

  http.begin(url);
  int httpCode = http.GET();

  if (httpCode > 0) {
    String payload = http.getString();
    Serial.printf("Antwort (%d): %s\n", httpCode, payload.c_str());
    isOn = !isOn;
  } else {
    Serial.printf("Fehler bei HTTP: %s\n", http.errorToString(httpCode).c_str());
  }

  http.end();
}
