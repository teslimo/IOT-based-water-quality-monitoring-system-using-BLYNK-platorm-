// Blynk credentials for cloud connectivity
// Must be defined before including libraries to avoid compilation issues



// Include necessary libraries
#include <U8g2lib.h>          // For controlling the OLED display
#include <Wire.h>             // For I2C communication (used by OLED and sensors)
#include <OneWire.h>          // For DS18B20 temperature sensor communication
#include <DallasTemperature.h> // For reading temperature from DS18B20
#include <WiFi.h>             // For ESP32 WiFi connectivity
#include <BlynkSimpleEsp32.h> // For Blynk cloud integration

// WiFi credentials for network connection
// Replace with your network's SSID and password
char ssid[] = "****";
char pass[] = "****";

// Initialize BlynkTimer for scheduling tasks (e.g., notifications)
BlynkTimer timer;

// Pin definitions organized in a namespace for clarity
namespace pin {
    const byte tds_sensor = 32;      // ADC pin for TDS (Total Dissolved Solids) sensor
    const byte turbidity_sensor = 35; // ADC pin for turbidity sensor
    const byte one_wire_bus = 4;     // Data pin for DS18B20 temperature sensor
    const byte clear_led = 13;       // LED indicating clear water (turbidity)
    const byte cloudy_led = 12;      // LED indicating cloudy water (turbidity)
    const byte dirty_led = 14;       // LED indicating dirty water (turbidity)
    const byte ph_sensor = 33;       // ADC pin for pH sensor
}

// Device settings namespace for hardware-specific parameters
namespace device {
    float aref = 3.3;  // ESP32 reference voltage (3.3V for ADC calculations)
}

// Sensor data namespace to store and manage sensor readings
namespace sensor {
    float ec = 0;                    // Electrical Conductivity (mS/cm)
    unsigned int tds = 0;            // Total Dissolved Solids (ppm)
    String tds_status = "";          // Status of TDS (Safe, Marginal, Unsafe)
    float ecCalibration = 1.0;       // Calibration factor for EC sensor
    int turbidity = 0;               // Turbidity level (NTU)
    String turbidity_status = "";    // Status of turbidity (Safe, Marginal, Unsafe)
    float celsius = 0;               // Temperature in Celsius
    float fahrenheit = 0;            // Temperature in Fahrenheit
    String temperature_status = "";  // Status of temperature (Safe, Unsafe)
    float phValue = 0;               // pH value
    int phSensorValue = 0;           // Raw ADC reading for pH sensor
    unsigned long int phAvgValue = 0;// Average of pH readings
    float phVol = 0;                 // Voltage from pH sensor
    int phBuf[10];                   // Buffer for pH sensor readings
    int phTemp;                      // Temporary variable for sorting pH readings
    String ph_status = "";           // Status of pH (Safe, Unsafe)
}

// OLED display setup (SH1106, 128x64 pixels, I2C communication)
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

// DS18B20 temperature sensor setup
OneWire oneWire(pin::one_wire_bus);        // Initialize OneWire bus on defined pin
DallasTemperature sensors(&oneWire);        // Initialize temperature sensor library

// Display state management for cycling through sensor screens
enum DisplayState { TDS_EC, TURBIDITY, TEMPERATURE, PH }; // States for display screens
DisplayState currentState = TDS_EC;                        // Current display state
unsigned long lastSwitchTime = 0;                          // Last time display switched
const unsigned long displayInterval = 3000;                // 3-second interval between screens

// Function to draw a specific sensor screen
void drawScreen(DisplayState state) {
    u8g2.setFont(u8g2_font_ncenB10_tr);  // Set bold font for sensor data
    char buffer[32];                     // Buffer for formatting text

    switch (state) {
        case TDS_EC: // Display TDS and EC screen
            u8g2.setFont(u8g2_font_unifont_t_symbols);  // Set font for icons
            u8g2.drawUTF8(0, 12, "\uE048");            // Draw droplet icon
            u8g2.setFont(u8g2_font_ncenB10_tr);        // Revert to bold font
            u8g2.drawStr(16, 12, "Water Quality");     // Title
            sprintf(buffer, "TDS: %u ppm", sensor::tds); // Format TDS value
            u8g2.drawStr(0, 30, buffer);               // Display TDS
            sprintf(buffer, "EC: %.2f mS/cm", sensor::ec); // Format EC value
            u8g2.drawStr(0, 44, buffer);               // Display EC
            u8g2.drawStr(0, 58, sensor::tds_status.c_str()); // Display TDS status
            break;
        case TURBIDITY: // Display Turbidity screen
            u8g2.setFont(u8g2_font_open_iconic_weather_2x_t); // Set font for icons
            u8g2.drawUTF8(0, 12, "\u0044");            // Draw water icon
            u8g2.setFont(u8g2_font_ncenB10_tr);        // Revert to bold font
            u8g2.drawStr(16, 12, "Turbidity");        // Title
            sprintf(buffer, "%d NTU", sensor::turbidity); // Format turbidity value
            u8g2.drawStr(0, 36, buffer);              // Display turbidity
            u8g2.drawStr(0, 50, sensor::turbidity_status.c_str()); // Display turbidity status
            break;
        case TEMPERATURE: // Display Temperature screen
            u8g2.setFont(u8g2_font_open_iconic_weather_2x_t); // Set font for icons
            u8g2.drawUTF8(0, 14, "\u0041");           // Draw thermometer icon
            u8g2.setFont(u8g2_font_ncenB10_tr);       // Revert to bold font
            u8g2.drawStr(16, 12, "Temperature");      // Title
            sprintf(buffer, "%.1f C", sensor::celsius); // Format Celsius
            u8g2.drawStr(0, 30, buffer);              // Display Celsius
            sprintf(buffer, "%.1f F", sensor::fahrenheit); // Format Fahrenheit
            u8g2.drawStr(0, 44, buffer);              // Display Fahrenheit
            u8g2.drawStr(0, 58, sensor::temperature_status.c_str()); // Display temp status
            break;
        case PH: // Display pH screen
            u8g2.setFont(u8g2_font_open_iconic_thing_2x_t); // Set font for icons
            u8g2.drawUTF8(0, 14, "\u0043");           // Draw beaker icon
            u8g2.setFont(u8g2_font_ncenB10_tr);       // Revert to bold font
            u8g2.drawStr(16, 12, "pH Level");        // Title
            sprintf(buffer, "pH: %.1f", sensor::phValue); // Format pH value
            u8g2.drawStr(0, 36, buffer);             // Display pH
            u8g2.drawStr(0, 50, sensor::ph_status.c_str()); // Display pH status
            break;
    }
}

// Function to send notifications when water parameters are unsafe
void notifyOnUnsafeParameters() {
    // Check if any sensor indicates unsafe conditions
    bool isUnsafe = sensor::tds_status == "Unsafe" || 
                    sensor::turbidity_status == "Unsafe" || 
                    sensor::temperature_status == "Unsafe" || 
                    sensor::ph_status == "Unsafe";

    if (isUnsafe) {
        char message[128];
        // Format notification message with all sensor values
        snprintf(message, sizeof(message), 
                 "Turbidity: %d NTU\nTDS: %u ppm\npH: %.1f\nTemperature: %.1f C\nWATER IS UNSAFE FOR CONSUMPTION ",
                 sensor::turbidity, sensor::tds, sensor::phValue, sensor::celsius);
        Serial.println("Notification Sent: ");
        Serial.println(message);
        // Send notification to Blynk app
        Blynk.logEvent("status", "Turbidity: " + String(sensor::turbidity) + " NTU\nTDS: " + String(sensor::tds) + " ppm\npH: " + String(sensor::phValue, 1) + "\nTemperature: " + String(sensor::celsius, 1) + " C\nWATER IS UNSAFE FOR CONSUMPTION");
    }
}

// Setup function to initialize hardware and connections
void setup() {
    // Start serial communication for debugging
    Serial.begin(115200);
    delay(1000);
    Serial.println("Water Quality Monitoring Started...");

    // Initialize OLED display
    u8g2.begin();
    u8g2.enableUTF8Print();  // Enable Unicode support for icons
    u8g2.setFont(u8g2_font_6x10_tr);  // Set smaller font for progress bar

    // Initialize DS18B20 temperature sensor
    sensors.begin();

    // Set pin modes for LEDs and pH sensor
    pinMode(pin::clear_led, OUTPUT);
    pinMode(pin::cloudy_led, OUTPUT);
    pinMode(pin::dirty_led, OUTPUT);
    pinMode(pin::ph_sensor, INPUT);

    // Connect to WiFi and Blynk
    Serial.println("Connecting to WiFi and Blynk...");
    Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

    // Schedule notifications every 5 seconds
    timer.setInterval(5000L, notifyOnUnsafeParameters);

    // Show startup progress bar animation
    showProgressBar();
}

// Main loop to handle sensor readings and display updates
void loop() {
    Blynk.run();  // Process Blynk communication
    timer.run();  // Run scheduled tasks (e.g., notifications)

    // Read all sensor data
    readTds();
    readTurbidity();
    readTemperature();
    readPh();

    // Handle display updates
    unsigned long currentTime = millis();
    if (currentTime - lastSwitchTime >= displayInterval) {
        u8g2.clearBuffer(); // Clear the display buffer
        drawScreen(currentState); // Draw the current screen
        u8g2.sendBuffer(); // Update the display
        currentState = static_cast<DisplayState>((currentState + 1) % 4); // Cycle to next state
        lastSwitchTime = currentTime; // Update the last switch time
    }
}

// Read and process TDS (Total Dissolved Solids) sensor data
void readTds() {
    float sum = 0;
    // Take 10 readings to average out noise
    for (int i = 0; i < 10; i++) {
        sum += analogRead(pin::tds_sensor);
        delay(10);
    }
    // Convert ADC reading to voltage
    float rawEc = (sum / 10) * device::aref / 4095.0;
    float offset = 0.14;  // Calibration offset for dry sensor
    sensor::ec = (rawEc * sensor::ecCalibration) - offset; // Calculate EC
    if (sensor::ec < 0) sensor::ec = 0; // Prevent negative EC
    // Convert EC to TDS using empirical formula
    sensor::tds = (133.42 * pow(sensor::ec, 3) - 255.86 * pow(sensor::ec, 2) + 857.39 * sensor::ec) * 0.5;

    // Determine TDS status based on thresholds
    if (sensor::tds <= 300) {
        sensor::tds_status = "Safe";
    } else if (sensor::tds <= 600) {
        sensor::tds_status = "Marginal";
    } else {
        sensor::tds_status = "Unsafe";
    }

    // Send TDS data to Blynk app
    Blynk.virtualWrite(V1, sensor::tds);
    Blynk.virtualWrite(V5, sensor::tds_status);

    // Log TDS data for debugging
    Serial.print("TDS: ");
    Serial.print(sensor::tds);
    Serial.print(" ppm, EC: ");
    Serial.print(sensor::ec, 2);
    Serial.print(" mS/cm, Status: ");
    Serial.println(sensor::tds_status);
}

// Read and process turbidity sensor data
void readTurbidity() {
    float sum = 0;
    // Take 10 readings to average out noise
    for (int i = 0; i < 10; i++) {
        sum += analogRead(pin::turbidity_sensor);
        delay(10);
    }
    int sensorValue = sum / 10; // Average ADC reading
    // Map ADC value to NTU (0-100 range)
    sensor::turbidity = map(sensorValue, 1300, 4095, 100, 0);
    sensor::turbidity = constrain(sensor::turbidity, 0, 100); // Limit to valid range

    // Set turbidity status and control LEDs
    if (sensor::turbidity < 1) {
        digitalWrite(pin::clear_led, HIGH);
        digitalWrite(pin::cloudy_led, LOW);
        digitalWrite(pin::dirty_led, LOW);
        sensor::turbidity_status = "Safe";
    } else if (sensor::turbidity <= 5) {
        digitalWrite(pin::clear_led, LOW);
        digitalWrite(pin::cloudy_led, HIGH);
        digitalWrite(pin::dirty_led, LOW);
        sensor::turbidity_status = "Marginal";
    } else {
        digitalWrite(pin::clear_led, LOW);
        digitalWrite(pin::cloudy_led, LOW);
        digitalWrite(pin::dirty_led, HIGH);
        sensor::turbidity_status = "Unsafe";
    }

    // Send turbidity data to Blynk app
    Blynk.virtualWrite(V0, sensor::turbidity);
    Blynk.virtualWrite(V4, sensor::turbidity_status);

    // Log turbidity data for debugging
    Serial.print("Turbidity ADC: ");
    Serial.print(sensorValue);
    Serial.print(" | Turbidity: ");
    Serial.print(sensor::turbidity);
    Serial.print(" NTU | Status: ");
    Serial.println(sensor::turbidity_status);
}

// Read and process temperature sensor data
void readTemperature() {
    sensors.requestTemperatures(); // Request temperature reading
    float rawCelsius = sensors.getTempCByIndex(0); // Read temperature
    sensor::celsius = rawCelsius; // Store in global variable
    sensor::fahrenheit = sensors.toFahrenheit(sensor::celsius); // Convert to Fahrenheit

    // Log raw and assigned values for debugging
    Serial.print("Raw Temp: ");
    Serial.print(rawCelsius);
    Serial.print(" C, Assigned Temp: ");
    Serial.print(sensor::celsius);
    Serial.println(" C");

    // Determine temperature status
    if (sensor::celsius >= 25 && sensor::celsius <= 30 && sensor::celsius != -127.0) {
        sensor::temperature_status = "Safe";
    } else {
        sensor::temperature_status = "Unsafe";
    }

    // Send valid temperature data to Blynk
    if (sensor::celsius != -127.0) { // -127.0 indicates sensor error
        Blynk.virtualWrite(V3, sensor::celsius);
        Blynk.virtualWrite(V6, sensor::temperature_status);
    }

    // Log temperature data for debugging
    Serial.print("Temperature: ");
    Serial.print(sensor::celsius);
    Serial.print(" C, ");
    Serial.print(sensor::fahrenheit);
    Serial.print(" F, Status: ");
    Serial.println(sensor::temperature_status);

    delay(100); // Small delay to stabilize reading
}

// Read and process pH sensor data
void readPh() {
    // Take 10 readings to reduce noise
    for (int i = 0; i < 10; i++) {
        sensor::phBuf[i] = analogRead(pin::ph_sensor);
        Serial.print("pH ADC = ");
        Serial.println(sensor::phBuf[i]);
        delay(10);  // 100ms total for readings
    }
    // Sort readings using bubble sort to remove outliers
    for (int i = 0; i < 9; i++) {
        for (int j = i + 1; j < 10; j++) {
            if (sensor::phBuf[i] > sensor::phBuf[j]) {
                sensor::phTemp = sensor::phBuf[i];
                sensor::phBuf[i] = sensor::phBuf[j];
                sensor::phBuf[j] = sensor::phTemp;
            }
        }
    }
    // Average middle 6 values to reduce noise
    sensor::phAvgValue = 0;
    for (int i = 2; i < 8; i++) {
        sensor::phAvgValue += sensor::phBuf[i];
    }
    // Convert average ADC to voltage
    sensor::phVol = (float)sensor::phAvgValue / 6 * device::aref / 4095.0;
    // Convert voltage to pH using calibration formula
    sensor::phValue = -5.70 * sensor::phVol + 21.34;

    // Constrain pH to realistic range (0-14)
    sensor::phValue = constrain(sensor::phValue, 0, 14);

    // Determine pH status
    if (sensor::phValue >= 6.5 && sensor::phValue <= 8.5) {
        sensor::ph_status = "Safe";
    } else {
        sensor::ph_status = "Unsafe";
    }

    // Send pH data to Blynk
    Blynk.virtualWrite(V2, sensor::phValue);
    Blynk.virtualWrite(V7, sensor::ph_status);

    // Log pH data for debugging
    Serial.print("pH Voltage: ");
    Serial.print(sensor::phVol);
    Serial.print(" V, pH: ");
    Serial.print(sensor::phValue);
    Serial.print(", Status: ");
    Serial.println(sensor::ph_status);
}

// Display a startup progress bar animation
void showProgressBar() {
    for (int percent = 1; percent <= 100; percent++) {
        u8g2.clearBuffer();
        u8g2.drawStr(20, 12, "Calibrating..."); // Display calibration message
        char buf[10];
        sprintf(buf, "%d%%", percent);          // Format percentage
        u8g2.drawStr(50, 24, buf);              // Display percentage
        u8g2.drawFrame(10, 30, 108, 10);        // Draw progress bar outline
        int barWidth = (percent * 106) / 100;   // Calculate bar width
        u8g2.drawBox(11, 31, barWidth, 8);      // Draw filled progress bar
        u8g2.sendBuffer();                      // Update display
        delay(40);                              // Delay for animation
    }
}
