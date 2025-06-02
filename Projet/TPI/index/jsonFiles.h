#ifndef JSONFILES_H
#define JSONFILES_H


// JSON file name in SPIFFS
const char* logFile1 = "/log1.json";                  //File log plant 1 
const char* logFile2 = "/log2.json";                  //File log plant 1
const char* plantInventory = "/plantInventory.json";  //Gile inventory plant 


// NTP server configuration
const char* ntpServer = "pool.ntp.org";       // Public time server
const long gmtOffset_sec = 3600;              // Timezone offset: +1 hour for UTC+1
const int daylightOffset_sec = 3600;          // Daylight saving time offset: +1h (use 0 to disable)

 //function to read the json logs 
  void readLogsJson1(WiFiClient& client) {
      
      //Open the file log.json and reads it 
      File file = SPIFFS.open(logFile1, FILE_READ);
      
      //error wile opening the file 
      if (!file || file.isDirectory()) {
        client.print("Document non complete");
        return;
      }
      
    //allocate a JSON document with capacity for 512 bytes
    StaticJsonDocument<512> doc;

    // aarse the JSON from the opened file into the document
    DeserializationError error = deserializeJson(doc, file);

    //if parsing failed, notify the client, close the file, and exit
    if (error) {
      client.print("Erreur de lecture JSON");
      file.close();
      return;
    }
    
      //Extract the value of the json file into variables
      const char* type = doc["type"];
      int humidity = doc["Humidity"];
      const char* date = doc["date"];
      int percent = doc["percent"];
      int timeWatering= doc["timeWatering"];

      //write the text on the server web 
      client.print(" Plante = ");
      client.print(type);
      client.print(", Humidite = ");
      client.print(humidity);
      client.print("%, Date = ");
      client.print(date);
      client.print(", Seuil d'humidite = ");
      client.print(percent);
      client.print("% ");
      client.print(", Temps d'arrosage = ");
      client.print(timeWatering);
      client.print(" secondes ");

      //close the file 
      file.close();
  }
    // Reads and prints the log content to the serial monitor
  void readLogsJson2(WiFiClient& client) {

      //Open the file log.json and reads it 
      File file = SPIFFS.open(logFile2, FILE_READ);
      
      //error wile opening the file 
      if (!file || file.isDirectory()) {
        client.print("Document non complete");
        return;
      }
    
    //allocate a JSON document with capacity for 512 bytes
    StaticJsonDocument<512> doc;

    // aarse the JSON from the opened file into the document
    DeserializationError error = deserializeJson(doc, file);

    //if parsing failed, notify the client, close the file, and exit
    if (error) {
      client.print("Erreur de lecture JSON");
      file.close();
      return;
    }

      //Extract the value of the json file into variables
      const char* type = doc["type"];
      int humidity = doc["Humidity"];
      const char* date = doc["date"];
      int percent = doc["percent"];
      int timeWatering= doc["timeWatering"];

     //write the text on the server web 
      client.print(" Plante = ");
      client.print(type);
      client.print(", Humidite = ");
      client.print(humidity);
      client.print("%, Date = ");
      client.print(date);
      client.print(" Seuil d'humidite = ");
      client.print(percent);
      client.print("% ");
      client.print(", Temps d'arrosage = ");
      client.print(timeWatering);
      client.print(" secondes ");

      //close the file 
      file.close();
  }


 #endif

  



