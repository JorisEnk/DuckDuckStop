#include <WiFi.h>
#include <WebServer.h>
#include <HTTPClient.h>
#include <Preferences.h>

Preferences prefs;
WebServer server(80);

// Flags
bool configMode = false;

// Globale Variablen für Konfiguration
String wifi_ssid, wifi_pass;
String shelly_ssid, shelly_pass, shelly_ip;

// Wiederholintervall
const unsigned long interval = 60 * 1000;
unsigned long lastRun = 0;

// HTML-Formular für Konfiguration
const char* form_html = R"rawliteral(
<html>
  <head><title>ESP32 Setup</title></head>
  <body>
    <h2>Konfiguration</h2>
    <form action="/save" method="get">
      <label>Internet SSID:</label><br>
      <input name="wifi_ssid"><br>
      <label>Internet Passwort:</label><br>
      <input name="wifi_pass"><br><br>
      <label>Shelly SSID:</label><br>
      <input name="shelly_ssid"><br>
      <label>Shelly Passwort:</label><br>
      <input name="shelly_pass"><br>
      <label>Shelly IP (z. B. 192.168.33.1):</label><br>
      <input name="shelly_ip"><br><br>
      <input type="submit" value="Speichern">
    </form>
  </body>
</html>
)rawliteral";

// ---------- SETUP ----------
void setup() {
  Serial.begin(115200);
  delay(1000);

  prefs.begin("config", false);

  // Konfiguration aus Speicher laden
  wifi_ssid = prefs.getString("wifi_ssid", "");
  wifi_pass = prefs.getString("wifi_pass", "");
  shelly_ssid = prefs.getString("shelly_ssid", "");
  shelly_pass = prefs.getString("shelly_pass", "");
  shelly_ip = prefs.getString("shelly_ip", "");

  if (wifi_ssid == "" || shelly_ssid == "" || shelly_ip == "") {
    Serial.println("⚠️  Keine Konfiguration gefunden. Starte Access Point.");
    startAccessPoint();
    configMode = true;
  } else {
    Serial.println("✅ Konfiguration geladen.");
  }
}

// ---------- LOOP ----------
void loop() {
  if (configMode) {
    server.handleClient();
    return;
  }

  unsigned long currentMillis = millis();
  if (currentMillis - lastRun >= interval || lastRun == 0) {
    lastRun = currentMillis;

    // Schritt 1: Mit Internet-WLAN verbinden
    connectToWiFi(wifi_ssid.c_str(), wifi_pass.c_str());

    if (WiFi.status() == WL_CONNECTED) {
      delay(1000);
      HTTPClient http;
      WiFiClient client;

      IPAddress dns = WiFi.dnsIP();
      Serial.print("DNS-Server: ");
      Serial.println(dns);

      http.begin(client, "http://example.com");
      int httpCode = http.GET();

      if (httpCode > 0) {
        String payload = http.getString();
        Serial.println("Antwort von example.com:");
        Serial.println(payload);
      } else {
        Serial.printf("Fehler bei HTTP: %s\n", http.errorToString(httpCode).c_str());
      }

      http.end();
    }

    delay(2000);

    // Schritt 2: Mit Shelly-WLAN verbinden
    connectToWiFi(shelly_ssid.c_str(), shelly_pass.c_str());

    if (WiFi.status() == WL_CONNECTED) {
      sendShellyCommand("on");
      delay(3000);
      sendShellyCommand("off");
    } else {
      Serial.println("❌ Verbindung zu Shelly fehlgeschlagen.");
    }

    Serial.println("⏳ Warte bis zum nächsten Durchlauf...\n");
  }

  delay(100);
}

// ---------- FUNKTIONEN ----------

void startAccessPoint() {
  WiFi.softAP("ESP32-Setup", "12345678");
  IPAddress IP = WiFi.softAPIP();
  Serial.print("Access Point gestartet. IP: ");
  Serial.println(IP);

  server.on("/", []() {
    server.send(200, "text/html", form_html);
  });

  server.on("/save", []() {
    wifi_ssid = server.arg("wifi_ssid");
    wifi_pass = server.arg("wifi_pass");
    shelly_ssid = server.arg("shelly_ssid");
    shelly_pass = server.arg("shelly_pass");
    shelly_ip = server.arg("shelly_ip");

    // Speichern
    prefs.putString("wifi_ssid", wifi_ssid);
    prefs.putString("wifi_pass", wifi_pass);
    prefs.putString("shelly_ssid", shelly_ssid);
    prefs.putString("shelly_pass", shelly_pass);
    prefs.putString("shelly_ip", shelly_ip);

    server.send(200, "text/html", "<h3>Gespeichert! ESP32 startet neu...</h3>");
    delay(2000);
    ESP.restart();
  });

  server.begin();
}

void connectToWiFi(const char* ssid, const char* password) {
  WiFi.disconnect(true);
  delay(500);
  WiFi.begin(ssid, password);
  Serial.printf("Verbinde mit WLAN: %s\n", ssid);

  int versuche = 0;
  while (WiFi.status() != WL_CONNECTED && versuche < 20) {
    delay(500);
    Serial.print(".");
    versuche++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n✅ Verbunden!");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\n❌ Verbindung fehlgeschlagen.");
  }
}

void sendShellyCommand(const String& action) {
  HTTPClient http;
  String url;

  if (action == "on") {
    url = String("http://") + shelly_ip + "/light/0?turn=on&mode=color&red=255&green=0&blue=0&gain=100";
  } else if (action == "off") {
    url = String("http://") + shelly_ip + "/light/0?turn=off&red=0&green=0&blue=0&gain=0";
  } else {
    Serial.println("Unbekannte Aktion!");
    return;
  }

  Serial.print("Sende: ");
  Serial.println(url);

  http.begin(url);
  int httpCode = http.GET();

  if (httpCode > 0) {
    Serial.printf("HTTP %d: %s\n", httpCode, http.getString().c_str());
  } else {
    Serial.printf("Fehler bei HTTP: %s\n", http.errorToString(httpCode).c_str());
  }

  http.end();
}
