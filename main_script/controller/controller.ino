#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>
#include <time.h>

// NTP
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 3600;
const int daylightOffset_sec = 3600;

Preferences preferences;
WebServer server(80);

// global configurations
String homeSSID, homePASS;
String shellyBulbSSID, shellyBulbPASS, shellyBulbIP;
String latitude, longitude;

// accessors
String getShellySSID() { return shellyBulbSSID; }
String getShellyPASS() { return shellyBulbPASS; }
String getShellyIP()   { return shellyBulbIP; }
String getHomeSSID()   { return homeSSID; }
String getHomePASS()   { return homePASS; }
String getLatitude()   { return latitude; }
String getLongitude()  { return longitude; }

// HTML form for configuration
const char* html_form = R"rawliteral(
<html>
  <head><title>ESP32 Setup</title></head>
  <body>
    <h2>WLAN-Konfiguration</h2>
    <form action="/save" method="get">
      <b>Heimnetzwerk</b><br>
      SSID: <input name="homeSSID"><br>
      Passwort: <input name="homePASS" type="password"><br><br>

      <b>Shelly Bulb</b><br>
      SSID: <input name="shellySSID"><br>
      Passwort: <input name="shellyPASS" type="password"><br>
      IP-Adresse: <input name="shellyIP"><br><br>

      <b>Standort</b><br>
      Breitengrad: <input name="latitude"><br>
      Längengrad: <input name="longitude"><br><br>

      <input type="submit" value="Speichern">
    </form>
  </body>
</html>
)rawliteral";

void setup() { 
  preferences.begin("wifi", false);
  Serial.begin(115200);
  delay(1000);

  if (!loadCredentials()) {
    Serial.println("no credentials found, starting AP mode");
    startAPMode();
    return;
  }

  WiFi.begin(homeSSID.c_str(), homePASS.c_str());
  Serial.printf("connecting to wifi: %s\n", homeSSID.c_str());
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWifi connected.");

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("access denied");
    return;
  }
  Serial.println("synchronized time");
}

void loop() {
  if (WiFi.getMode() == WIFI_AP) {
    server.handleClient();
    return;
  }

  float price = getCurrentPrice(); // Dummy / Placeholder
  float radiation = getCurrentRadiation();
  float grid = getForecastLoad(); // Dummy / Placeholder
  float wind = getAverageWindNext8Hours();

  Serial.printf("\nWind: %.2f km/h, Radiation: %.2f W/m², Grid: %.2f MW\n", wind, radiation, grid);

  int r, g, b;
  berechneRGB(radiation, wind, grid, &r, &g, &b);
  setShellyBulbColor(r, g, b);

  delay(60 * 1000);
}

void startAPMode() {
  WiFi.softAP("ESP32-Setup", "12345678");
  IPAddress ip = WiFi.softAPIP();
  Serial.println("Access Point started: http://" + ip.toString());

  server.on("/", []() {
    server.send(200, "text/html", html_form);
  });

  server.on("/save", []() {
    homeSSID = server.arg("homeSSID");
    homePASS = server.arg("homePASS");
    shellyBulbSSID = server.arg("shellySSID");
    shellyBulbPASS = server.arg("shellyPASS");
    shellyBulbIP = server.arg("shellyIP");
    latitude = server.arg("latitude");
    longitude = server.arg("longitude");

    preferences.begin("wifi", false);
    preferences.putString("homeSSID", homeSSID);
    preferences.putString("homePASS", homePASS);
    preferences.putString("shellySSID", shellyBulbSSID);
    preferences.putString("shellyPASS", shellyBulbPASS);
    preferences.putString("shellyIP", shellyBulbIP);
    preferences.putString("latitude", latitude);
    preferences.putString("longitude", longitude);
    preferences.end();

    server.send(200, "text/html", "<h2>Gespeichert! Neustart...</h2>");
    delay(2000);
    ESP.restart();
  });

  server.onNotFound([]() {
    server.send(404, "text/plain", "Not Found");
  });

  server.begin();
}

bool loadCredentials() {
  preferences.begin("wifi", true);
  homeSSID       = preferences.getString("homeSSID", "");
  homePASS       = preferences.getString("homePASS", "");
  shellyBulbSSID = preferences.getString("shellySSID", "");
  shellyBulbPASS = preferences.getString("shellyPASS", "");
  shellyBulbIP   = preferences.getString("shellyIP", "");
  latitude       = preferences.getString("latitude", "");
  longitude      = preferences.getString("longitude", "");
  preferences.end();

  return (homeSSID.length() > 0 && shellyBulbSSID.length() > 0 && shellyBulbIP.length() > 0 &&
          latitude.length() > 0 && longitude.length() > 0);
}

float constrainFloat(float value, float min, float max) {
  if (value < min) return min;
  if (value > max) return max;
  return value;
}

void berechneRGB(float radiation, float wind, float grid, int* r, int* g, int* b) {
  float rad_norm = constrainFloat(radiation / 1000.0, 0.0, 1.0);
  float wind_norm = constrainFloat((wind - 10.0) / 80.0, 0.0, 1.0);
  float grid_norm = constrainFloat((grid - 3500.0) / 3500.0, 0.0, 1.0);

  if (rad_norm < 0.05 && wind_norm < 0.05 && grid_norm < 0.05) {
    *r = 255;
    *g = 255;
    *b = 0;
    return;
  } else {
    float qualitaet = 0.5 * rad_norm + 0.5 * wind_norm;
    *b = 0;
    *r = (int)(255.0 * fabs(grid_norm - qualitaet));
    *g = (int)(255.0 * qualitaet);
  }
}
