import subprocess
import matplotlib.pyplot as plt
from datetime import datetime
import matplotlib.dates as mdates
import re

# Run the external script and capture the output
result = subprocess.run(
    ["python3", "get_energyprice_data.py"],
    capture_output=True,
    text=True
)

# Extract the relevant lines
lines = result.stdout.strip().splitlines()

timestamps = []
prices = []

for line in lines:
    match = re.match(r"(\d{4}-\d{2}-\d{2} \d{2}:\d{2}) - \d{2}:\d{2} : ([\d\.\-]+)", line)
    if match:
        dt_str, price_str = match.groups()
        dt = datetime.strptime(dt_str, "%Y-%m-%d %H:%M")
        price = float(price_str)
        timestamps.append(dt)
        prices.append(price)

# Plot
plt.figure(figsize=(12, 6))
plt.plot(timestamps, prices, marker="o")
plt.title("Strompreise von Awattar (€/MWh)")
plt.xlabel("Zeit")
plt.ylabel("Preis (€/MWh)")
plt.grid(True)
plt.xticks(rotation=45)
plt.gca().xaxis.set_major_formatter(mdates.DateFormatter("%m-%d %H:%M"))
plt.tight_layout()
plt.axhline(0, color='gray', linestyle='--', linewidth=1)
plt.savefig('strompreise_plot.png')  # Save as a PNG image