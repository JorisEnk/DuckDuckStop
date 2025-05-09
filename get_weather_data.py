import requests
import pandas as pd
from datetime import date, timedelta

# coordinates Freiburg
lat, lon = 47.9990, 7.8421

today = date.today()
end = today + timedelta(days=7)

url = (
    f"https://api.open-meteo.com/v1/forecast?"
    f"latitude={lat}&longitude={lon}"
    "&hourly=temperature_2m,cloudcover,direct_radiation,diffuse_radiation"
    f"&start_date={today}&end_date={end}"
    "&timezone=Europe%2FBerlin"
)

data = requests.get(url).json()
df = pd.DataFrame(data['hourly'])
df['time'] = pd.to_datetime(df['time'])

weather_info = {}

for i, row in df.iterrows():
    time = row['time'].isoformat()
    direct = row['direct_radiation']
    cloud = row['cloudcover']
    temp = row['temperature_2m']
    diffuse = row['diffuse_radiation']
    
    # simple rules for sunlight and action
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

    # dict
    weather_info[time] = {
        "direct_radiation": direct,
        "diffuse_radiation": diffuse,
        "temperature": temp,
        "cloudcover": cloud,
        "sunlight": sunlight,
        "sun_strength": strength,
        "recommended_action": action
    }

# save data for today
day = today.isoformat()
#print(f"\nWetterdaten für {day}:")
for t in list(weather_info):
    if t.startswith(day):
        print(f"{t} -> {weather_info[t]}")
# save to json file
with open(f"weather_data_{day}.json", "w") as f:
    f.write(str(weather_info))