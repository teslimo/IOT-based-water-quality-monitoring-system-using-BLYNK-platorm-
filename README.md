# IoT-Based Water Quality Monitoring System with Blynk

## 📖 Introduction

This project is an **IoT-based Water Quality Monitoring System** that
measures key parameters of water in real-time and sends the data to
**Blynk**, allowing remote monitoring through a smartphone or web
dashboard. The system is designed to ensure safe and reliable water
usage in homes, agriculture, and industries by providing continuous
insights into water quality.

------------------------------------------------------------------------

## 🔎 Project Overview

-   The system uses sensors to measure water quality parameters.\
-   Data is collected by an **ESP32/ESP8266 microcontroller**.\
-   Readings are transmitted to the **Blynk IoT platform** for real-time
    monitoring.\
-   Users can view the data on the **Blynk mobile app** or dashboard.\
-   Alerts and notifications can be set for unsafe water conditions.

------------------------------------------------------------------------

## ✨ Key Features

-   📡 **IoT Connectivity** -- Remote monitoring via Blynk app.\
-   🌡️ **Real-time Parameter Measurement** -- Water quality indicators
    continuously updated.\
-   🔔 **Threshold Alerts** -- Notifications when readings exceed safe
    limits.\
-   📱 **Mobile & Web Dashboard** -- Access readings from anywhere.\
-   🔋 **Low Power Consumption** -- Optimized for long-term monitoring.

------------------------------------------------------------------------

## ⚙️ How It Works

1.  Water quality sensors are immersed in the water sample.\
2.  The microcontroller (ESP32/ESP8266) reads the sensor data.\
3.  Data is processed and sent to the **Blynk IoT cloud** over Wi-Fi.\
4.  Users monitor the data in real-time through the **Blynk
    app/dashboard**.\
5.  Alerts are triggered if parameters exceed predefined safety levels.

------------------------------------------------------------------------

## 🧩 Components Used

  -----------------------------------------------------------------------
  Component              Description                 Function
  ---------------------- --------------------------- --------------------
  **ESP32 / ESP8266**    Wi-Fi microcontroller       Collects sensor data
                                                     & sends to Blynk

  **pH Sensor**          Measures acidity/alkalinity Ensures water pH is
                                                     within safe range

  **Turbidity Sensor**   Measures water clarity      Detects suspended
                                                     particles

  **Temperature Sensor   Measures water temperature  Helps analyze water
  (DS18B20/DHT11)**                                  quality trends

  **TDS Sensor**         Measures dissolved solids   Indicates
                                                     mineral/salt
                                                     concentration

  **Blynk IoT Platform** Cloud dashboard             Displays real-time
                                                     water quality data

  **Power Supply**       5V/3.3V regulated           Powers the system
  -----------------------------------------------------------------------

------------------------------------------------------------------------

## 🖥️ Software & Libraries

-   **Arduino IDE / PlatformIO** for coding and uploading firmware.\
-   **Blynk Library** for cloud integration.\
-   **ESP32/ESP8266 Wi-Fi Library** for connectivity.\
-   **Sensor libraries** (e.g., OneWire, DallasTemperature, etc.).

------------------------------------------------------------------------

## 🚀 Setup Instructions

1.  Install **Arduino IDE** and required libraries.\

2.  Clone this repository:

    ``` bash
    git clone https://github.com/yourusername/IoT-Water-Quality-System.git
    ```

3.  Open the code in Arduino IDE.\

4.  Enter your **Wi-Fi credentials** and **Blynk Auth Token** in the
    code.\

5.  Upload the code to the ESP32/ESP8266.\

6.  Open the **Blynk app**, add widgets (gauges, graphs,
    notifications).\

7.  Start monitoring water quality in real-time!

------------------------------------------------------------------------

## 📊 Example Dashboard (Blynk)

-   pH Value\
-   Water Turbidity\
-   TDS Level\
-   Temperature\
-   Alert Notifications

------------------------------------------------------------------------

## 📌 Applications

-   Drinking water monitoring.\
-   Agriculture (irrigation water quality).\
-   Aquaculture and fish farming.\
-   Industrial water quality control.\
-   Smart cities and IoT labs.

------------------------------------------------------------------------

## 📷 Project Images / Demo

*(Add your circuit diagram, prototype, or app screenshots here)*

------------------------------------------------------------------------

## 📜 License

This project is open-source and available under the **MIT License**.
