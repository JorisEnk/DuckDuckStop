import requests
from datetime import datetime, timedelta, timezone

# 🔧 Eingabewerte
zip_code = "79100"
date_from = datetime.utcnow().date()
date_to = date_from + timedelta(days=1)

# API-URL bauen
url = f"https://api.stromgedacht.de/v1/forecast?zip={zip_code}&from={date_from}&to={date_to}"
print("URL:", url)

# API-Request
response = requests.get(url)
print("Status Code:", response.status_code)

try:
    data = response.json()
except Exception as e:
    print("Fehler beim Parsen:", e)
    print(response.text)
    exit(1)

# Zeitgrenze: jetzt bis 2 Stunden später (UTC, timezone-aware)
now = datetime.now(timezone.utc)
future_limit = now + timedelta(hours=3)

# Forecast filtern
load_data = data.get("load", [])
filtered = []

for entry in load_data:
    dt_str = entry.get("dateTime")
    dt = datetime.fromisoformat(dt_str.replace("Z", "+00:00"))
    if now <= dt <= future_limit:
        filtered.append((dt_str, entry.get("value")))

# Gefilterte Werte anzeigen
if not filtered:
    print("⚠️ Keine Daten im gewünschten Zeitfenster gefunden.")
else:
    print(f"\n⚡ Netzlast für die nächsten 2 Stunden (UTC):")
    for dt_str, value in filtered:
        print(f"{dt_str} - {value} MW")

    # Mittelwert berechnen
    values = [val for _, val in filtered]
    avg = sum(values) / len(values)
    threshold = 5500  # Schwellwert in MW

    print(f"\n📊 Durchschnittliche Netzlast: {avg:.2f} MW")

    if avg > threshold:
        print("🚨 Auslastung: HOCH")
    else:
        print("✅ Auslastung: NICHT hoch")
