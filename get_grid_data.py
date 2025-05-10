import requests

# 🔧 Eingabewerte
zip_code = "79100"
hours = 2

# URL bauen
url = f"https://api.stromgedacht.de/v1/now?zip={zip_code}&hoursInFuture={hours}"
print("URL:", url)

# Anfrage senden
response = requests.get(url)

try:
    data = response.json()
except Exception as e:
    print("Fehler beim Parsen der JSON-Antwort:", e)
    print(response.text)
    exit(1)

# Status abrufen
state = data.get("state")

if state is not None:
    print(f"\n Stromampel-Status für PLZ {zip_code} in den nächsten {hours} Stunden:")
    print(f"Zustand: {state}")
else:
    print(" Kein Statuswert in der API-Antwort gefunden.")
