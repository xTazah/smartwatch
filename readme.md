# ⌚ Smartwatch Prototype with Base Station

## 🛠️ Overview
This project is a prototype for a "smartwatch" implemented using Arduino. The system includes various sensors and a base station for enhanced monitoring and visualization.

### 🌟 Features:
- 📉 **Sensors:** 
  - Gyroscope
  - Water
  - Humidity
  - Temperature
- ⏱️ **Actuator:** Real-Time Clock (RTC) Display
- 📊 **Data Storage and Visualization:**
  - Sensor values are stored in an **InfluxDB** database.
  - Visualized on a **Grafana** dashboard.
- 🖥️ **Base Station:**
  - Displays selected sensor values for direct monitoring.

---

## 💡 How It Works
1. **Smartwatch Functionality:**
   - Sensors collect environmental and motion data.
   - Data is transmitted wirelessly to a base station via **CoAP** (Constrained Application Protocol).
   - Real-Time Clock (RTC) display shows the current time.

2. **Base Station:**
   - Receives sensor data.
   - Displays key information like temperature, humidity, and gyroscope readings.

3. **Data Pipeline:**
   - Sensor data is sent to an **InfluxDB** instance.
   - Data is processed and visualized in a **Grafana** dashboard.

---

## 🧰 Technologies Used
### 🎛️ Hardware
- Arduino-based **ESP32** microcontroller for the smartwatch.
- Various sensors for environmental and motion tracking.
- Base station hardware for monitoring.

### 🌐 Communication
- **CoAP (Constrained Application Protocol)**: Lightweight communication protocol for IoT devices.
- **WiFi**: For data transmission between the smartwatch and base station.

### 📊 Data Handling
- **InfluxDB**: Stores sensor data for historical analysis.
- **Grafana**: Visualizes data with customizable dashboards.

### 💻 Software
- Arduino **C++** code manages sensors and communication.
- **Python** backend for handling CoAP requests, data processing, and database integration.

---

## 🚀 Workflow
1. Smartwatch initializes and connects to WiFi.
2. Sensors collect data and send it to the base station using CoAP.
3. Python backend processes sensor data:
   - Stores it in InfluxDB.
   - Updates the Grafana dashboard.
4. Base station displays real-time values like temperature and humidity.
5. Smartwatch RTC syncs with the Python backend to maintain accurate time.

---