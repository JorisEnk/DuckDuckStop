#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// WLAN-Zugangsdaten
const char* ssid = "TP-Link_F574";
const char* password = "38433948";

// IP-Adresse der Shelly Plug S Gen3
const char* shelly_ip = "192.168.1.102";

// Intervall in Millisekunden (z. B. 10 Sekunden)
const unsigned long interval = 10 * 1000;
unsigned long lastRequest = 0;

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

  // Steckdose sicherheitshalber einschalten
  ensureShellyOn();
}

void loop() {
  unsigned long currentMillis = millis();

  if (currentMillis - lastRequest >= interval) {
    lastRequest = currentMillis;
    readShellyPowerData();
  }

  delay(100); // Entlastung
}

void ensureShellyOn() {
  HTTPClient http;
  String url = String("http://") + shelly_ip + "/rpc/Switch.Set?id=0&on=true";
  http.begin(url);
  int httpCode = http.GET();
  if (httpCode == 200) {
    Serial.println("🔌 Shelly wurde eingeschaltet.");
  } else {
    Serial.printf("❌ Fehler beim Einschalten: %s\n", http.errorToString(httpCode).c_str());
  }
  http.end();
}

void readShellyPowerData() {
  HTTPClient http;
  String url = String("http://") + shelly_ip + "/rpc/Switch.GetStatus?id=0";

  http.begin(url);
  int httpCode = http.GET();

  if (httpCode == 200) {
    String payload = http.getString();
    DynamicJsonDocument doc(2048);
    DeserializationError error = deserializeJson(doc, payload);

    if (!error) {
      float power = doc["apower"];
      float voltage = doc["voltage"];
      float current = doc["current"];

      Serial.println("📊 Shelly Gen3 Messwerte:");
      Serial.printf("  🔌 Leistung : %.2f W\n", power);
      Serial.printf("  ⚡ Spannung : %.2f V\n", voltage);
      Serial.printf("  🔋 Strom    : %.3f A\n\n", current);
    } else {
      Serial.println("❌ Fehler beim Parsen der Messdaten");
    }
  } else {
    Serial.printf("❌ HTTP Fehler: %d\n", httpCode);
  }

  http.end();
}
