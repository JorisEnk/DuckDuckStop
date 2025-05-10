#if defined(ESP8266)
  #include <ESP8266WiFi.h>
  #include <ESP8266HTTPClient.h>
#else
  #include <WiFi.h>
  #include <HTTPClient.h>
#endif

// WLAN-Zugangsdaten
const char* ssid     = "TP-Link_F574";
const char* password = "38433948";

// IP-Adresse des Lesekopfs
const char* host     = "192.168.1.109";

void setup() {
  Serial.begin(115200);
  delay(1000);

  // WLAN verbinden
  Serial.print("Verbinde mit WLAN: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  // Auf Verbindung warten
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWLAN verbunden!");
  Serial.print("Lokale IP: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    // Ziel-URL (liefert HTML mit Messdaten)
    String url = String("http://") + host + "/?m=1";
    http.begin(url);

    Serial.println("Sende Anfrage an Lesekopf...");

    // HTTP GET-Anfrage
    int httpCode = http.GET();

    if (httpCode > 0) {
      // HTTP-Code anzeigen
      Serial.print("HTTP Code: ");
      Serial.println(httpCode);

      // Antwort empfangen
      String payload = http.getString();
      Serial.println("Messdaten (HTML):");
      Serial.println("------------------------------------");
      Serial.println(payload);
      Serial.println("------------------------------------");

    } else {
      Serial.print("Fehler bei HTTP-Anfrage: ");
      Serial.println(http.errorToString(httpCode));
    }

    http.end();
  } else {
    Serial.println("WLAN getrennt. Versuche erneut zu verbinden...");
    WiFi.begin(ssid, password);
  }

  delay(10000); // Alle 10 Sekunden neue Abfrage
}
