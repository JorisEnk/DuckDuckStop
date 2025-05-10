import requests
import pandas as pd
from datetime import datetime, timedelta
import json

# location dynamically determined by IP address
def get_location():
    try:
        response = requests.get("https://ipinfo.io/json")
        data = response.json()
        lat, lon = map(float, data["loc"].split(","))
        print(f"Standort ermittelt: {lat}, {lon}")
        print(f"Standort: {data['city']}, {data['region']}, {data['country']}")
        return lat, lon
    except:
        print("Standort konnte nicht automatisch ermittelt werden. Fallback: Freiburg.")
        return 47.9990, 7.8421

# next 12h weather data
now = datetime.now()
now_rounded = now.replace(minute=0, second=0, microsecond=0)  # auf volle Stunde runden
end = now_rounded + timedelta(hours=12)

lat, lon = get_location()

# Open-Meteo API URL
url = (
    f"https://api.open-meteo.com/v1/forecast?"
    f"latitude={lat}&longitude={lon}"
    "&hourly=temperature_2m,cloudcover,direct_radiation,diffuse_radiation"
    f"&start_date={now_rounded.date()}&end_date={(end + timedelta(days=1)).date()}"
    "&timezone=Europe%2FBerlin"
)

data = requests.get(url).json()
df = pd.DataFrame(data['hourly'])
df['time'] = pd.to_datetime(df['time'])

# only keep data for the next 12h
df = df[(df['time'] >= now_rounded) & (df['time'] <= end)]

weather_info = {}

for i, row in df.iterrows():
    time = row['time'].isoformat()
    direct = row['direct_radiation']
    cloud = row['cloudcover']
    temp = row['temperature_2m']
    diffuse = row['diffuse_radiation']

    if direct == 0:
        sunlight = False
        strength = "none"
        action = "idle"
    elif direct < 200:
        sunlight = True
        strength = "low"
        action = "store"
    elif direct < 500:
        sunlight = True
        strength = "medium"
        action = "store"
    else:
        sunlight = True
        strength = "high"
        action = "feed_in"

    weather_info[time] = {
        "direct_radiation": direct,
        "diffuse_radiation": diffuse,
        "temperature": temp,
        "cloudcover": cloud,
        "sunlight": sunlight,
        "sun_strength": strength,
        "recommended_action": action
    }

# Output
print(f"\nWetterdaten von {now_rounded} bis {end}:\n")
for t in sorted(weather_info.keys()):
    print(f"{t} -> {weather_info[t]}")

with open("weather_forecast_24h.json", "w") as f:
    json.dump(weather_info, f, indent=2)
