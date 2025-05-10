#include <WiFi.h>
#include <HTTPClient.h>

// WLAN-Daten
const char* ssid = "shellycolorbulb-4091515826AA";
const char* password = "";  // leer lassen, falls Shelly ohne Passwort läuft

// Shelly IP
const char* shelly_ip = "192.168.33.1";

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

  // Lampe auf Rot setzen und einschalten
  setColorRed();
}

void loop() {
  // nichts tun
}

void setColorRed() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    // Lampe einschalten und Farbe auf Rot setzen
    String url = String("http://") + shelly_ip + "/light/0?turn=on&red=0&green=255&blue=0";

    http.begin(url);
    int httpCode = http.GET();

    if (httpCode > 0) {
      Serial.printf("Lampe auf Rot gesetzt. HTTP Code: %d\n", httpCode);
    } else {
      Serial.printf("Fehler beim Setzen der Farbe: %s\n", http.errorToString(httpCode).c_str());
    }

    http.end();
  } else {
    Serial.println("Nicht mit WLAN verbunden!");
  }
}
