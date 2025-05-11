#include <WiFi.h>
#include <time.h>

// WLAN
const char* ssid = "Häck2025";
const char* password = "B#campus!";

// NTP
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 3600;
const int daylightOffset_sec = 3600;

// Grenzwerte
const float priceLimit = 150.0;        // €/MWh
const float radiationLimit = 200.0;    // W/m²

// Externe Funktionen
extern float getCurrentPrice();
extern float getCurrentRadiation();
extern void toggleShelly(bool state);
extern void toggleShellyBulb(bool state);
extern float getForecastLoad();
extern float getAverageWindNext8Hours();

void setup() {
  Serial.begin(115200);
  delay(1000);

  // WLAN verbinden
  WiFi.begin(ssid, password);
  Serial.print("🔌 Verbinde mit WLAN");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n✅ WLAN verbunden.");

  // Zeit synchronisieren
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("❌ Keine Zeit verfügbar");
    return;
  }
  Serial.println("🕒 Zeit synchronisiert");
}

// Hilfsfunktion zum Beschränken eines Werts zwischen min und max
float constrainFloat(float value, float min, float max) {
  if (value < min) return min;
  if (value > max) return max;
  return value;
}

void berechneRGB(float radiation, float wind, float grid, int* r, int* g, int* b) {
// Normierungen
float rad_norm = constrainFloat(radiation / 1000.0, 0.0, 1.0);
float wind_norm = constrainFloat((wind - 10.0) / 80.0, 0.0, 1.0);
float grid_norm = constrainFloat((grid - 3500.0) / 3500.0, 0.0, 1.0);

// Sonderfall: Alles schlecht → Rot
if (rad_norm < 0.05 && wind_norm < 0.05 && grid_norm < 0.05) {
    *r = 255;
    *g = 255;
    *b = 0;
    return;
}else{
  float qualitaet = 0.5 * rad_norm + 0.5 * wind_norm;
     // RGB-Berechnung
    *b = 0;
    *r = (int)(255.0 * fabs(grid_norm - qualitaet));
    *g = (int)(255.0 * qualitaet);
}
}
void runRGBTests() {
  struct TestSet {
    float radiation;
    float wind;
    float grid;
  };

  TestSet testValues[] = {
    {50,  5,  6000},
    {300, 20, 5500},
    {700, 50, 4500},
    {1000, 80, 3000},
    {0,    0,  7000}
  };

  const int numTests = sizeof(testValues) / sizeof(TestSet);

  for (int i = 0; i < numTests; i++) {
    float rads = testValues[i].radiation;
    float wind = testValues[i].wind;
    float grid = testValues[i].grid;

    int r, g, b;
    berechneRGB(rads, wind, grid, &r, &g, &b);

    Serial.printf("\n🧪 Test %d\n☀️ %.1f W/m², 🌬️ %.1f km/h, ⚡ %.1f MW\n", i + 1, rads, wind, grid);
    Serial.printf("🎨 RGB: R=%d G=%d B=%d\n", r, g, b);

    setShellyBulbColor(r, g, b);

    delay(15000);  // 15 Sekunden Pause
  }

  Serial.println("\n✅ Alle RGB-Tests abgeschlossen.");
}

void runRGBTests() {
  struct TestSet {
    float radiation;
    float wind;
    float grid;
  };

  TestSet testValues[] = {
    // Beste Bedingungen: hohe Strahlung, hoher Wind, gutes Netz
    {1000, 90, 7000},
    // Nur hohe Strahlung
    {1000, 0, 3500},
    // Nur hoher Wind
    {0, 90, 3500},
    // Nur gutes Netz
    {0, 0, 7000},
    // Durchschnittliche Bedingungen
    {500, 40, 3500},
    // Hohe Netzlast, gute EE
    {800, 60, 1000},
    // Alles maximal schlecht
    {0, 0, 0},
    // Extrem hoher Netzoutput
    {1000, 80, 8000},
    // Negative Werte (zur Validierung der constrain-Funktion)
    {-50, -10, -500},
    // Werte exakt auf den Schwellen
    {50, 10, 3500}
  };

  const int numTests = sizeof(testValues) / sizeof(TestSet);

  for (int i = 0; i < numTests; i++) {
    float rads = testValues[i].radiation;
    float wind = testValues[i].wind;
    float grid = testValues[i].grid;

    int r, g, b;
    berechneRGB(rads, wind, grid, &r, &g, &b);

    Serial.printf("\n🧪 Test %d\n☀️ %.1f W/m², 🌬️ %.1f km/h, ⚡ %.1f MW\n", i + 1, rads, wind, grid);
    Serial.printf("🎨 RGB: R=%d G=%d B=%d\n", r, g, b);

    setShellyBulbColor(r, g, b);

    delay(15000);  // 15 Sekunden Pause
  }
}

void loop() {
  runRGBTests();

  while (true) {
    delay(1000);  // Tests nur einmal ausführen
  }
}


