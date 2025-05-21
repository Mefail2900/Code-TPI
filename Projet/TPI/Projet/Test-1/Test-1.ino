#include <WiFi.h>              // Manages Wi-Fi connection on ESP32
#include <SPIFFS.h>            // Handles internal SPIFFS file system
#include <ArduinoJson.h>       // Allows creation, reading, and writing of JSON files
#include <time.h>              // Provides access to time via NTP

// Wi-Fi credentials (replace with your network info)
const char* ssid = "NETGEAR69";
const char* password = "fancyflower620";

// JSON file name in SPIFFS
const char* logFile = "/log.json";

// NTP server configuration
const char* ntpServer = "pool.ntp.org";       // Public time server
const long gmtOffset_sec = 3600;              // Timezone offset: +1 hour for UTC+1
const int daylightOffset_sec = 3600;          // Daylight saving time offset: +1h (use 0 to disable)

void setup() {
  Serial.begin(115200);
  delay(500);

  // -------------------------------------------------- Wi-Fi connection
  WiFi.begin(ssid, password);
  Serial.print("Connecting to Wi-Fi : ");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.print("Local IP address: ");
  Serial.println(WiFi.localIP());

  // -------------------------------------------------- Mount SPIFFS
  if (!SPIFFS.begin(false)) {
    Serial.println(" Failed to mount SPIFFS!");
    return;
  }

  // -------------------------------------------------- NTP Time retrieval
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to get time from NTP server");
    return;
  }

  // Format the current date and time as a string
  char dateTime[20];
  strftime(dateTime, sizeof(dateTime), "%Y-%m-%d %H:%M", &timeinfo);

  // Create a JSON document with a single entry
  DynamicJsonDocument doc(512);
  doc["name"] = "Mint";
  doc["type"] = "Aromatic";
  doc["Humidity"] = 52;
  doc["date"] = dateTime;
  doc["pin"] = 26;
  

  // -------------------------------------------------- Write JSON to file
  File file = SPIFFS.open(logFile, "w");
  serializeJsonPretty(doc, file);
  file.close();

  // Read and print the content of the JSON file
  readLog();
}

void loop() {
  // Read and print the log repeatedly (for test/debug purposes)
  readLog();
  delay(3000);
}

// Reads and prints the log content to the serial monitor
void readLog() {
  File file = SPIFFS.open(logFile, "r");
  if (!file) {
    Serial.println("File not found!");
    return;
  }

  Serial.println("=== Latest log entry ===");
  while (file.available()) {
    Serial.write(file.read());
  }
  Serial.println("\n=== End ===");
  file.close();
}
