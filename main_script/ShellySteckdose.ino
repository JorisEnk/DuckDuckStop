#include <HTTPClient.h>

const char* shelly_ip = "192.168.1.102";

void toggleShelly(bool state) {
  HTTPClient http;
  String action = state ? "on" : "off";
  String url = String("http://") + shelly_ip + "/relay/0?turn=" + action;

  Serial.printf("Steckdose %s...\n", action.c_str());
  http.begin(url);
  int httpCode = http.GET();

  if (httpCode > 0) {
    Serial.printf("Antwort: %s\n", http.getString().c_str());
  } else {
    Serial.printf("Fehler: %s\n", http.errorToString(httpCode).c_str());
  }

  http.end();
}
