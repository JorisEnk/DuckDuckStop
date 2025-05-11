# Optimizing Small-Scale Storage and Plug-In Solar for Grid Stability and Cost Savings

## Problem Statement  
Small-scale plug-in solar systems with battery storage are often not optimized to support grid stability. Prosumers typically charge batteries based on electricity prices rather than real-time grid conditions, potentially contributing to grid stress during peak solar production. Currently, there are no simple dynamic incentives or guidance for prosumers to adjust behavior to align with grid needs, resulting in inefficiencies and missed opportunities to use excess renewable energy.

## Hackathon Challenge  
**How might we enable prosumers to optimize their energy use and feed-in behavior to support grid stability, lower CO₂ emissions, and increase financial benefits—while ensuring the solution is simple, affordable, and scalable?**

## Data & Tools Used
- Real-time electricity price and grid demand APIs  
- Weather forecast data for solar generation prediction  
- Battery & inverter data (where accessible)  
- Smart home hardware (e.g. Shelly bulbs, ESP32, sensors)  
- Home Assistant + AppDaemon (early prototype phase)

## Our Journey & Learnings

We initially focused on developing an automated solution using **Home Assistant** with **AppDaemon**, integrating smart home sensors, grid data, and solar predictions to control battery feed-in behavior. However, we encountered several technical and practical limitations:

- **Hardware issues**: The WISPR smart meter failed to provide usable data despite extensive setup.  
- **Closed ecosystems**: Our battery (Marstek Jupiter C) did not expose data beyond its proprietary app.  
- **Scalability challenges**: Requiring Home Assistant and custom scripting made the solution too complex and niche for the average user.  
- **Risk of over-automation**: Fully automatic control removes user agency and may behave poorly in unpredictable situations.

Following feedback and brainstorming sessions—including an insightful discussion with Sebastian—we pivoted toward a more **accessible, low-tech, and user-centric** approach.

## Final Solution: Visual Feed-In Recommendations via ESP32 & Smart Bulbs

We developed a lightweight, accessible system using a **low-cost ESP32 microcontroller** and **two Shelly smart bulbs**. Here’s how it works:

1. **ESP32 Access Point**: On first use, it creates a Wi-Fi access point for users to enter home network credentials.
2. **Bulb Indicators**:
   - 🟢 **Green**: Feed into the grid now – high demand, good prices.
   - 🔴 **Red**: Do not feed – excess supply or low demand.
   - 🟡 **Yellow**: Neutral – consider user preferences or battery status.
3. **Smart Algorithm on ESP32**: It pulls real-time data from grid APIs and weather forecasts to determine optimal feed-in timing based on demand and renewable energy availability, customized for the user's location.

### Why This Works
- **Simplicity**: No complex home automation system required  
- **Affordability**: ESP32 and Shelly bulbs are inexpensive and widely available  
- **User agency**: Clear visual signals guide behavior without removing control  
- **Scalability**: Easily replicable and adaptable for various home setups  
- **Open Source**: Fully documented and modifiable for community development

## Setup Instructions

To get started with the ESP32 + Shelly bulb system, follow these steps:

### 1. Clone This Repository

```bash
git clone https://github.com/JorisEnk/DuckDuckStop.git
```

### 2. Flash the ESP32

1. Open the Arduino IDE.
2. Connect your ESP32 via USB.
3. Navigate to `main_script/controllter/controller.ino` in this repo.
4. Select the correct ESP32 board under **Tools > Board** (e.g., “ESP32 Dev Module”).
5. Choose the correct COM port under **Tools > Port**.
6. Click **Upload** to flash the firmware to your ESP32.

>  *If you're new to ESP32 setup in Arduino IDE, refer to [this guide](https://randomnerdtutorials.com/installing-the-esp32-board-in-arduino-ide-windows-instructions/).*

### 3. Connect ESP32 to Your Home Wi-Fi

- After flashing, the ESP32 will start as a **Wi-Fi access point** (e.g., `ESP32-Setup`).
- Connect your phone or laptop to that Wi-Fi (password: `12345678`).
- A captive portal or setup page should appear automatically. If not, go to `http://192.168.4.1`.
- Enter:
  - Your **home Wi-Fi SSID and password**
  - **Shelly Bulb IP address or SSID**:  
    Usually `192.168.33.1` by default. If this doesn't work, please refer to the official **Shelly Bulb documentation** or your local network settings for the correct address.

Click **Connect**.

### 4. Watch the Bulb React
Once connected to the internet, the ESP32 will begin pulling real-time grid and weather data. The Shelly smart bulb will reflect the current feed-in recommendation through simple color-coded signals.

>  *This lightweight setup gives you real-time visual feedback without needing an app, Home Assistant, or other automation platforms.*

