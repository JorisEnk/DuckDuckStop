import requests
import datetime

def get_awattar_prices(start=None, end=None):
    base_url = "https://api.awattar.de/v1/marketdata"
    params = {}

    if start:
        params["start"] = int(start.timestamp() * 1000)  # in Millisekunden
    if end:
        params["end"] = int(end.timestamp() * 1000)

    response = requests.get(base_url, params=params)

    if response.status_code != 200:
        print("Fehler beim Abrufen der Daten:", response.status_code)
        return

    data = response.json()
    prices = data.get("data", [])

    print("📈 Strompreise von Awattar (€/MWh):\n")
    for entry in prices:
        start_time = datetime.datetime.fromtimestamp(entry["start_timestamp"] / 1000)
        end_time = datetime.datetime.fromtimestamp(entry["end_timestamp"] / 1000)
        price = entry["marketprice"]
        print(f"{start_time.strftime('%Y-%m-%d %H:%M')} - {end_time.strftime('%H:%M')} : {price:.2f} €/MWh")

if __name__ == "__main__":
    # Optional: Zeitraum definieren (z. B. heute + morgen)
    now = datetime.datetime.now() - datetime.timedelta(days=2)
    tomorrow = now + datetime.timedelta(days=2)

    get_awattar_prices(start=now, end=tomorrow)
