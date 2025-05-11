// Script to get control the Shelly Bulb
#include <WiFi.h>
#include <HTTPClient.h>

extern String shellyBulbSSID;
extern String shellyBulbPASS;
extern String homeSSID;
extern String homePASS;
extern String shellyBulbIP;


void connectToShellyBulbWiFi() {
  Serial.println("Connect to ShellyBulb WiFi...");
  Serial.println("SSID: " + shellyBulbSSID);
  Serial.println("PASS: " + shellyBulbPASS);

  WiFi.disconnect(true);
  delay(500);
  WiFi.begin(shellyBulbSSID.c_str(), shellyBulbPASS.c_str());

  for (int i = 0; i < 20 && WiFi.status() != WL_CONNECTED; i++) {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n Connected to ShellyBulb");
  } else {
    Serial.println("\n Connection to ShellyBulb failed");
  }
}


void connectToHomeWiFi() {
  Serial.println("Connect to home WiFi...");
  Serial.println("SSID: " + homeSSID);
  Serial.println("PASS: " + homePASS);

  Serial.println("Connecting to home WiFi...");
  WiFi.disconnect(true);
  delay(500);
  WiFi.begin(homeSSID.c_str(), homePASS.c_str());

  for (int i = 0; i < 20 && WiFi.status() != WL_CONNECTED; i++) {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n back to home WiFi");
  } else {
    Serial.println("\n Connection to home WiFi failed");
  }
}

void setShellyBulbColor(int r, int g, int b) {
  connectToShellyBulbWiFi();

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println(" no connection to ShellyBulb WiFi");
    connectToHomeWiFi();
    return;
  }

  HTTPClient http;

  // set RGB color
  String url = String("http://") + shellyBulbIP +
               "/light/0?turn=on&mode=color&red=" + String(r) +
               "&green=" + String(g) +
               "&blue=" + String(b) +
               "&gain=100";  // or: "&brightness=100" je nach Firmware

  Serial.println("send to ShellyBulb: " + url);
  http.begin(url);
  int httpCode = http.GET();

  if (httpCode > 0) {
    Serial.printf("RGB set: R=%d G=%d B=%d (HTTP %d)\n", r, g, b, httpCode);
  } else {
    Serial.printf("Error setting the color: %s\n", http.errorToString(httpCode).c_str());
  }

  http.end();

  connectToHomeWiFi();
}


// Glühbirne schalten
void toggleShellyBulb(bool state) {
  connectToShellyBulbWiFi();

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("no connection to ShellyBulb");
    return;
  }

  HTTPClient http;
  String url = String("http://") + shellyBulbIP + "/light/0?turn=" + (state ? "on" : "off");

  http.begin(url);
  int httpCode = http.GET();

  if (httpCode > 0) {
    Serial.printf("ShellyBulb set: %s (HTTP %d)\n", state ? "an" : "aus", httpCode);
  } else {
    Serial.printf("error setting ShellyBulb: %s\n", http.errorToString(httpCode).c_str());
  }

  http.end();

  connectToHomeWiFi();  // return to home WiFi
}