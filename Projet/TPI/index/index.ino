/*********
  Mefail Sulejmani 

  Title: Automatic watering 

  introduction: This code is designed to monitor plant conditions and water them either manually via a smartphone or automatically when the soil humidity drops below a threshold set through the web interface.

  Code : This code connects the ESP32-WROOM to Wi-Fi, hosts a web server displaying real-time humidity data, watering logs,
  and controls buttons to manually or automatically water, add, modify, or delete up to two plants, with all data stored in the ESP32’s memory.

*********/

#include <WiFi.h>            // Wi-Fi library
#include <SPIFFS.h>          // Filesystem for internal storage on the memory of the ESP32 library
#include <ArduinoJson.h>     // JSON handling library
#include <time.h>            // get the time  library
#include "humidity.h"        //Page for he humidity values 
#include "jsonFiles.h"       //Page for the json files 
   

   

// Replace with your network credentials
const char* ssid = "NETGEAR69";
const char* password = "fancyflower620";

// Set web server port number to 80
WiFiServer server(80);

// Variable to store the HTTP request
String header;



String typePlant1;  //get the type of the plant 1 put on the json file 
String typePlant2;  //get the type of the plant 2 put on the json file 


const int output27 = 27;    //to activate the pump for the plant 1 
const int output26 = 26;    //to activate the pump for the plant 2


bool plant1 ;     //For checking if the plant 1 is created 
bool plant2 ;     //For checking if the plant 2 is created 



int levelWateringPlant1 ;   //get the percent of the plant 1 put on the json file   
int levelWateringPlant2 ;   //get the percent of the plant 2 put on the json file  

int timeWatering = 8000;    // the time for watering starts with 8 seconds default ,  1000 = 1 second 


unsigned long currentTime = millis();     // Stores the current time in milliseconds since the board started

unsigned long previousTime = 0;            // Will be updated when a new client connects

const long timeoutTime = 2000;              // Maximum time (2 seconds) to wait for a client to send the full HTTP request


//This function check if the plant 1 and 2 exist 
void checkPlantInventory() {
  
  //put always the value on false 
  plant1 = false;
  plant2 = false;

  //Open the plant inventory json file and reads it 
  File file = SPIFFS.open("/plantInventory.json", "r");

  //return if the file exist 
  if (!file || file.isDirectory()) return;

  //Creates a JSON document in memory with a size of 1024 byte
  StaticJsonDocument<1024> doc;
  //reads the JSON file and if the file contains invalid structure JSON it will return false 
  DeserializationError error = deserializeJson(doc, file);

  //close the file
  file.close();

  //if a error pups out the  function exits early 
  if (error) return;

  //converts the json document on array for structuring better the code 
  JsonArray array = doc.as<JsonArray>();

  //Loop to check if the id 1 and 2 exist on the file if yes it will turn the vaiables true 
  for (JsonObject plant : array) {
    int id = plant["ID"];
    if (id == 1) plant1 = true;
    if (id == 2) plant2 = true;
  }
}

//function were the code will start 
void setup() {


  Serial.begin(115200);     // Start serial communication at 115200 baud


  
  pinMode(output27, OUTPUT);      // Set pin 25 as output for pump 1
  pinMode(output26, OUTPUT);      // Set pin 26 as output for pump 2



  digitalWrite(output27, HIGH);        // Ensure pump 1 is off
  digitalWrite(output26, HIGH);        // Ensure pump 2 is off

   // Connect to Wi-Fi network with SSID and password
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  //call the function to 
  wifiConnection();

  
  //checks if the spiffs works 
  if (!SPIFFS.begin(false)) {
    Serial.println("Failed to mount SPIFFS!");
    return;
  }

  //call the function to check if the plants exist
  checkPlantInventory(); 
  

}

//this function is made because it will be a loop if the esp32 can't connect to the WIFI
void wifiConnection() {
  //numer to try connect
  int numberTry = 0;

  //he will try to connect 2 sec 
  while (WiFi.status() != WL_CONNECTED && numberTry < 2) {
    //1 second
    delay(1000);
    Serial.print(".");

    //incrise the numberTry by 1
    numberTry++;
  }

  //check if the esp32 is connected to the wifi
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nConnected to the WIFI");
    Serial.print("Adress IP : ");
    Serial.println(WiFi.localIP());
    server.begin();
  } 

}


//This functions create and change the functions on the server Web 
void serverWeb(){
   WiFiClient client = server.available();   // Listen for incoming clients

    //Call the functions for the humidity for refreshing 
    moistureSensor1();
    moistureSensor2();

         // If a new client connects
        if (client) {        
          //record the time of the server start                     
          currentTime = millis();

          //change the time on the current time 
          previousTime = currentTime;

          String currentLine = "";                // make a String to hold incoming data from the client
          
          // loop while the client's connected
          while (client.connected() && currentTime - previousTime <= timeoutTime) {  
            currentTime = millis();

            // if there is a data to read from the client
            if (client.available()) {  

              char c = client.read();             // read a byte, then
            
            //delete part 
              header += c;


        //handle adding a new plant when POST /addPlant is received  from the web page 
        if (header.indexOf("POST /addPlant") >= 0) {
        
        //skip all HTTP headers from the POST request
        while (client.available()) {
          String line = client.readStringUntil('\n');
          if (line.length() <= 2) break;
        }

        //read the body of the forum   from the POST request
        String body = "";
        while (client.available()) body += (char)client.read();

        //extract the type on percent 
        String type = "", percentStr = "";
 
        //Convert the type and the percent on variables 
        int typePos = body.indexOf("type=");
        int percentPos = body.indexOf("percent=");


        //if the input of the percent and the time is present 
        if (typePos != -1 && percentPos != -1) {
           
          //get the type and divide on 2 parts with the &  
          type = body.substring(typePos + 5, body.indexOf("&"));
          
          //converts only the numbers 
          percentStr = body.substring(percentPos + 8);

          //this rapresent space on HTML DATA 
          type.replace('+', ' ');
        }

        //convert percent string to int and cal the ficntion to create the plant 
        int percent = percentStr.toInt();
        createPlant(type, percent);  // call with value 

        //Respond with a redirect to the homepage
        client.println("HTTP/1.1 303 See Other");
        client.println("Location: /");
        client.println("Connection: close");
        client.println();
        break;
      }


      //Handle modify the plant 1 when POST /modifyPlant1 is received  from the web page
      if (header.indexOf("POST /modifyPlant1") >= 0) {
       
        //Skip all HTTP headers until the blank line
        while (client.available() && client.read()!='\n');
       
        //read the full request body
        String body = "";

        //read the entire HTTP request body into the body string
        while (client.available()) body += char(client.read());

        // Helper lambda to extract a parameter value from the body , ai help
        auto getParam = [&](const String& key){
          
          //find the position of key in the body, p is in the index of the first character
          int p = body.indexOf(key + "=");

          // If key is not found (indexOf returned -1), return an empty Strin
          if (p < 0) return String();

          //Find the position of the next '&' after the key, marking the end of the value
          int e = body.indexOf('&', p);

          // Extract and return the substring starting just after "key="
          return body.substring(p + key.length()+1, e<0?body.length():e);
        };

        //call getParam with the type to extract the new plant type from the request body
        String newType    = getParam("type");

        //convert the results on integer on the new variable
        int    newPercent = getParam("percent").toInt();

        //call the function modifyPlantByID and adds the values 
        modifyPlantByID(1, newType, newPercent);
        
        //Respond with a redirect to the homepage
        client.println("HTTP/1.1 303 See Other");
        client.println("Location: /");
        client.println("Connection: close");
        client.println();
        break;
      }

      //Handle modify the plant 2 when POST /modifyPlant1 is received  from the web page
      if (header.indexOf("POST /modifyPlant2") >= 0) {

        //Skip all HTTP headers until the blank line
        while (client.available() && client.read()!='\n');

        //read the full request body
        String body = "";

        //read the entire HTTP request body into the body string
        while (client.available()) body += char(client.read());

        // Helper lambda to extract a parameter value from the body , ai help
        auto getParam = [&](const String& key){

          //find the position of key in the body, p is in the index of the first character
          int p = body.indexOf(key + "=");

          // If key is not found (indexOf returned -1), return an empty String
          if (p < 0) return String();

          //Find the position of the next '&' after the key, marking the end of the value
          int e = body.indexOf('&', p);

          // Extract and return the substring starting just after "key=
          return body.substring(p + key.length()+1, e<0?body.length():e);
        };

        //call getParam with the type to extract the new plant type from the request body
        String newType    = getParam("type");

         //convert the results on integer on the new variable
        int    newPercent = getParam("percent").toInt();
        
        //call the function modifyPlantByID and adds the values 
        modifyPlantByID(2, newType, newPercent);
        
        //Respond with a redirect to the homepage
        client.println("HTTP/1.1 303 See Other");
        client.println("Location: /");
        client.println("Connection: close");
        client.println();
        break;
      }

   

        
        //check if the if the request of the sensor humidity 1 is sent 
        if (header.indexOf("GET /moisture1") >= 0) {

          //read the moisture 1 %
          moistureSensor1();      
                     
          //Send  HTTP status line and headers  response     
          client.println("HTTP/1.1 200 OK");
          client.println("Content-type:text/plain");
          client.println("Connection: close\n");

           // % of the moisture sensor
          client.print(moisturePercent1);                  

          break;
        }

        //check if the if the request of the sensor humidity 2 is sent 
        if (header.indexOf("GET /moisture2") >= 0) {

          //read the moisture 2 %
          moistureSensor2();                          
          
          //Send  HTTP status line and headers  response   
          client.println("HTTP/1.1 200 OK");
          client.println("Content-type:text/plain");
          client.println("Connection: close\n");
            
          // % of the moisture sensor 
          client.print(moisturePercent2); 
          break;
        }
        //check if the if the request of the water pump 1 is sent 
        if (header.indexOf("GET /water1") >= 0) {

          //desactivate the pin and the pumps activate 
          digitalWrite(output27, LOW);
         
          
          //stops the program until the delay is finished
          delay(timeWatering);

          //activate the pin so the pump desactivate
          digitalWrite(output27, HIGH); 
          
          //Write the blush logs in the Json file to keep track 
          WriteLogJson1();

          //Send  HTTP status line and headers, succes response with ou body  
          client.println("HTTP/1.1 204 No Content"); 
          client.println("Connection: close\n");
          break;
        }
        //check if the if the request of the water pump 2 is sent
        if (header.indexOf("GET /water2") >= 0) {

          //desactivate the pin and the pumps activate 
          digitalWrite(output26, LOW);
          
          //stops the program until the delay is finished
          delay(timeWatering);
          
          //activate the pin so the pump desactivate
          digitalWrite(output26, HIGH);
          
          //Write the blush logs in the Json file to keep track 
          WriteLogJson2();

          //Send  HTTP status line and headers, succes response with ou body
          client.println("HTTP/1.1 204 No Content"); // réponse vide
          client.println("Connection: close\n");

          break;
        }

        //check if the if the request of the time is sent 
        if (header.indexOf("POST /timeWater") >= 0) {

     //Read and discard  all HTTP headers until  the blank line that separates headers from the body
      while (client.available()) {
        String line = client.readStringUntil('\n');
        if (line.length() <= 2) break;
      }

      //read the full request body
      String body = "";

      //
      while (client.available()) body += (char)client.read();

      //converts the number on integer
      int pos = body.indexOf("newTime=");

      //If pos is not -1, the parameter was found
      if (pos != -1) {
        
        //converts the caracter on string with 8 caracters long 
        String value = body.substring(pos + 8);

        //convert the time into integer 
        int time = value.toInt();
        
        //checks if the number add is between 0 and 60 
        if (time >= 1 && time <= 60) {
         
          //convert seconds in milliseconds 
          timeWatering = time * 1000; 
  
        } else {
          Serial.println("value off limit");
        }
      }

      //Send  HTTP status line and headers, redirection 
      client.println("HTTP/1.1 303 See Other");
      client.println("Location: /");
      client.println("Connection: close");
      client.println();
      break;
    }


        // check if the byte is a newline character
        if (c == '\n') {                   
          
          // We’ve received a newline; check if it’s the end of the HTTP request headers
          if (currentLine.length() == 0) {


                //Handle to delete the plant by id request if present in the request line
                if (header.indexOf("GET /deletePlant?id=") >= 0) {

                //convert to the number by skipping the 3 letters id= 
                int idPos   = header.indexOf("id=") + 3;

                //convert the id wih a space 
                int idEnd   = header.indexOf(' ', idPos);

                //convert the id into integrer 
                int plantId = header.substring(idPos, idEnd).toInt();

                //call  the function for deleting the plant 
                deletePlantByID(plantId);

                //check the inventory plant 
                checkPlantInventory();


                //Send  HTTP status line and headers, redirection 
                client.println("HTTP/1.1 303 See Other");
                client.println("Location: /");
                client.println("Connection: close");
                client.println();
                break; }

                // or he will generate a normal html reponse 
                client.println("HTTP/1.1 200 OK");
                client.println("Content-type:text/html");
                client.println("Connection: close");
                client.println();

             // Display the HTML web page
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            
            // CSS to style of most of the site  
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { background-color: #4CAF50; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println(".moisture1Text{font-size:3rem;color:#28a745;font-weight:bold;margin:0;}");
            client.println(".moisture2Text{font-size:3rem;color:#28a745;font-weight:bold;margin:0;}");
            client.println(".button {background-color: green;}");
            client.println(".buttonRed {background-color: red;color: white;} </style></head>");
            
            // Web Page Heading
            client.println("<body><h1>ESP32 Web Server</h1>");
            
           //time for watering 
            client.print("<p>Le temps d'arrosage est de ");
            client.print(timeWatering / 1000, 1);   // it will devide the number to get the time in seconds 
            client.println(" secondes</p>");
           
           //input for changing the time blushing 
            client.println("<form action=\"/timeWater\" method=\"POST\">");
            client.print("<label for=\"newTime\">Temps d'arrosage</label>");
            client.print("<input type=\"number\" id=\"newTime\" name=\"newTime\" min=\"1\" max=\"60\" required>");
            client.print("<input type=\"submit\" value=\"Changer\" style=\"margin-left: 10px;\">");
            client.print("</form>");

            //check if the 2 plants exist 
             if(!plant1 || !plant2 ){
            
            //show the forms to add a plant 
            client.println("<h3>Nouvelle plante</h3>");
            client.println("<form action='/addPlant' method='POST'>");
            client.println("<label for='type'>Plante</label>");
            client.println("<input type='text' id='type' name='type' required>");
            client.println("<label for='type'> Pourcentage de cycles arrosage actives</label>");
            client.println("<input type='number' id='percent'  name='percent' required><br><br>");
            client.println("<input type='submit' value='Charger' style='padding:6px 16px;'>");
            client.println("</form></div>");
          }
          else{
            client.println("<h3>Maximum de plante ajoute</h3>");
            
          }
        
          //show the plant 1 if exist 
          if(plant1){
            //border of the box 
            client.println("<div style=\"border:2px solid black; padding:10px; border-radius:6px;\">");

            client.println("<h2>Plante 1</h2>");
            client.println("<h4>Humidite</h4>");
            
            //humidity of the plant 
            client.print("<p id=\"humidity1\" class=\"moisture1Text\">");
            client.print(moisturePercent1 );
            client.print("%</p>");
  
            //Show the button "Arroser" on green, if the user click the button the color and the text of the button wil change on red and the text will be "Arrosage" made AI 
            //it refresh automaticly without refreshing the page 
            client.println(
              "<p><button style='background:#4CAF50;color:#fff;padding:16px 40px;font-size:30px;border:none;cursor:pointer'"
              " onclick=\"(b=>{b.innerText='Arrosage';b.style.background='red';b.disabled=true;"
              "fetch('/water1').then(()=>{b.innerText='Arroser';b.style.background='#4CAF50';b.disabled=false;refresh();});})(this)\">"
              "Arroser</button></p>"
            );
  
            //read the plant 1 on the json file 
            readInventoryPlant1(client);
            client.print("<p>Le dernier arrosage de la plante 1 est :  ");

            //reads the history of plant 1 
            readLogsJson1(client);
            client.println(" </p></div>");
           
          }
          //show the plant 1 if exist 
          if(plant2){
            
            //border of the box 
            client.println("<div style=\"border:2px solid black; padding:10px; border-radius:6px;\">");

            client.println("<h2>Plante 2</h2>");
            client.println("<h4>Humidite</h4>");
            
            //humidity of the plan
            client.print("<p id=\"humidity2\" class=\"moisture2Text\">");
            client.print(moisturePercent2);
            client.print("%</p>");
  
            //Show the button "Arroser" on green, if the user click the button the color and the text of the button wil change on red and the text will be "Arrosage" made AI 
            //it refresh automaticly without refreshing the page
            client.println(
              "<p><button style='background:#4CAF50;color:#fff;padding:16px 40px;font-size:30px;border:none;cursor:pointer'"
              " onclick=\"(b=>{b.innerText='Arrosage';b.style.background='red';b.disabled=true;"
              "fetch('/water2').then(()=>{b.innerText='Arroser';b.style.background='#4CAF50';b.disabled=false;refresh();});})(this)\">"
              "Arroser</button></p>"
            );

            //read the plant 1 on the json file 
            readInventoryPlant2(client);
            client.print("<p>Le dernier arrosage de la plante 2 est :  ");

            //reads the history of plant 1 
            readLogsJson2(client);
            client.println(" </p></div>");
          }
          if(!plant1 && !plant2){
            client.println("<h3>Aucune plante ajoute </h3>");
          }


            //get a refresh of the sensor humidity 
            moistureSensor1();
            moistureSensor2();

          //java script function for refreshing the values of the humidiy with out refreshing the page 
           client.println("<script>");
          client.println("function refresh1(){");
          client.println("  fetch('/moisture1').then(r=>r.text()).then(v=>document.getElementById('humidity1').textContent = v + '%');");
          client.println("}");
          client.println("function refresh2(){");
          client.println("  fetch('/moisture2').then(r=>r.text()).then(v=>document.getElementById('humidity2').textContent = v + '%');");
          client.println("}");
          client.println("setInterval(refresh1, 1000);");
          client.println("setInterval(refresh2, 1000);");
          client.println("</script>");

          // The HTTP response ends with another blank line
          client.println();
          break;
          }
          // if you got a newline, then clear currentLine
          else { 
            currentLine = "";
          }
        }
        // if you got anything else but a carriage return character,
        else if (c != '\r') { 
          // add it to the end of the currentLine
          currentLine += c;      
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();

  }
}



//principal loop were the code will turn 
void loop(){

    //if the esp32 is connected open the webserver 
    if (WiFi.status() == WL_CONNECTED) {
      
      //updates the web page every loop 
       serverWeb();

    }
    //if the esp32 is not connected 
    if(WiFi.status() != WL_CONNECTED){

      //open and read the values of the plant 
      loadPlantVariablesOffline();

      //read the humidity of the 2 plants 
       moistureSensor1();
       moistureSensor2();

       Serial.print("non internettt");

    }
 
  //checks if the plant 1 exist
  if(plant1)automaticWateringPlant1();
  
  //checks if the plant 2 exist
  if(plant2) automaticWateringPlant2();
  
}


///save the logs of the plant 1 
void WriteLogJson1(){

  //get the time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  struct tm timeinfo;

  //check if the ntp server works 
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to get time from NTP server");
    return;
  }

  //convert the time for watering 
  int durationSec1 = timeWatering / 1000;

  // insert the date on a chart variables with year, month, day, hour and mintutes 
  char dateTime[20];
  strftime(dateTime, sizeof(dateTime), "%Y-%m-%d %H:%M", &timeinfo);
    
  // Create a JSON document with a capacity of 512 bytes
  DynamicJsonDocument doc(512);

    //Converts the value and saved on the file 
    doc["ID"] = 1;
    doc["type"] = typePlant1;
    doc["Humidity"] = moisturePercent1;
    doc["date"] = dateTime;
    doc["percent"] = levelWateringPlant1;
    doc["timeWatering"] = durationSec1;
    

    //Write on the JSON file
    File file = SPIFFS.open(logFile1, "w");
    serializeJsonPretty(doc, file);
    
    //close the file 
    file.close();
  }
  

//This function checks if the humidity level is right of the plant 1 
void automaticWateringPlant1(){

  //This condition is made to check if the level of the humidity is correct
  if (moisturePercent1 < levelWateringPlant1 ) {

    //desactivate the pin so the pump can be activated  
    digitalWrite(output27, LOW);
    
    //if the esp32 is connected to the wifi write the logs 
    if (WiFi.status() == WL_CONNECTED) {
      
      //call the function to write the log on the json file 
       WriteLogJson1();
    }
  }
  else{
    //activate the pin so the pump can be desactivated 
     digitalWrite(output27, HIGH);
  }

  //if the wifi is not connected 
  if (WiFi.status() != WL_CONNECTED) {
    
    //try to connect to the wifi 
    wifiConnection();
  }
}

//save the logs of the plant 2
void WriteLogJson2(){

    //get the time
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    struct tm timeinfo;

    //check if the ntp server works 
    if (!getLocalTime(&timeinfo)) {
      Serial.println("Failed to get time from NTP server");
      return;
    }
    //convert the time for watering
    int durationSec2 = timeWatering / 1000;

    // insert the date on a chart variables with year, month, day, hour and mintutes 
      char dateTime[20];
    strftime(dateTime, sizeof(dateTime), "%Y-%m-%d %H:%M", &timeinfo);
      
    // Create a JSON document with a capacity of 512 bytes
    DynamicJsonDocument doc(512);

      doc["ID"] = 2;
      doc["type"] = typePlant2;
      doc["Humidity"] = moisturePercent2;
      doc["date"] = dateTime;
      doc["percent"] = levelWateringPlant2;
      doc["timeWatering"] = durationSec2;

      //Write on the JSON file
      File file = SPIFFS.open(logFile2, "w");
      serializeJsonPretty(doc, file);
      
      //close the file 
      file.close();
  }

//This function checks if the humidity level is right of the plant 2
void automaticWateringPlant2(){

  //This condition is made to check if the level of the humidity is correct
  if (moisturePercent2 < levelWateringPlant2) {

    //desactivate the pin so the pump can be activated  
    digitalWrite(output26, LOW);

    //if the esp32 is connected to the wifi write the logs 
   if (WiFi.status() == WL_CONNECTED) {
      
    //call the function to write the log on the json file 
    WriteLogJson2();
    }
  }
  else{

    //activate the pin so the pump can be desactivated  
     digitalWrite(output26, HIGH);
  }
    //if the wifi is not connected 
  if (WiFi.status() != WL_CONNECTED) {
    
    //try to connect to the wifi 
    wifiConnection();
  }
}


//function made for checking the type and the percent of the water with out connection 
void loadPlantVariablesOffline() {
  
   // Ouvre le fichier JSON contenant les données des plantes en lecture
  File file = SPIFFS.open("/plantInventory.json", FILE_READ);

  //checks if the file can be open 
  if (!file || file.isDirectory()) {
    Serial.println("Impossible to open the JSON file.");
    return;
  }

  //create a dynamic JSON document in memory with 1024 bytes of space
  StaticJsonDocument<1024> doc;
  DeserializationError error = deserializeJson(doc, file);
  file.close();

  //Get the main JSON array from the document
  JsonArray array = doc.as<JsonArray>();

  //loop through each object (plant) in the JSON array
  for (JsonObject plant : array) {
    int id = plant["ID"]; // Get the ID of the plant

    //checks if the plant id 1 exist 
    if (id == 1) {
      //converts values
      typePlant1 = plant["type"].as<String>();
      levelWateringPlant1 = plant["percent"].as<int>();
    }
    //checks if the plant id 2 exist 
    if (id == 2) {
      //converts values
      typePlant2 = plant["type"].as<String>();
      levelWateringPlant2 = plant["percent"].as<int>();
    }
  }
}

//refresh the values of the type and percent by id 
void modifyPlantByID(int idToModify, const String& newType, int newPercent) {
 
  //open the json file and reads it 
  File file = SPIFFS.open("/plantInventory.json", "r");

  //if the file doesnt exist 
  if (!file || file.isDirectory()) return;

  //Creates a JSON document in memory with a size of 1024 byte
  DynamicJsonDocument doc(1024);

  //checks the inventory and replace the values by id 
  DeserializationError err = deserializeJson(doc, file);
  
  //close the file
  file.close();

  //retur the DeserializationError if an error came 
  if (err || !doc.is<JsonArray>()) return;

  //serch the array on the json file 
  JsonArray array = doc.as<JsonArray>();

  // serch teh  objet on on the array and modify the vaule by the nuew value 
  for (JsonObject plant : array) {
    if (plant["ID"] == idToModify) {
      plant["type"]    = newType;
      plant["percent"] = newPercent;
      break;
    }
  }
  // open and write on the json file 
  file = SPIFFS.open("/plantInventory.json", "w");
  if (!file) return;

  //save the updated JSON array back to the file in a readable format
  serializeJsonPretty(array, file);

  //close the file 
  file.close();

  // refresh the value of the plant inventory 
  checkPlantInventory();

}
//function to create the the plant 
void createPlant(const String& type, int percent) {

  // Create a small JSON document for the new plant
  DynamicJsonDocument newPlant(256);
  
  // Store the plant type and the percent 
  newPlant["type"] = type;
  newPlant["percent"] = percent;

   //assign a unique Id use 1 if first slot free, otherwise use 2
  if (!plant1) {
    newPlant["ID"] = 1;
    plant1 = true;
  } else if (!plant2) {
    newPlant["ID"] = 2;
    plant2 = true;
  }

  // Load the existing inventory into a larger JSON document
  DynamicJsonDocument doc(1024);

  //read  the plant inventory file 
  File file = SPIFFS.open("/plantInventory.json", "r");
  
  //if it exist 
  if (file && !file.isDirectory()) {
    DeserializationError error = deserializeJson(doc, file);
    file.close();
    //if parse fails or  document isn't an array, reset to an empty array
    if (error || !doc.is<JsonArray>()) {
      doc = DynamicJsonDocument(1024);
      doc.to<JsonArray>();
    }
  } else {
    // no an existing file starts with an arrow 
    doc.to<JsonArray>();
  }

  // Add the new plant object to the array
  doc.as<JsonArray>().add(newPlant);

  //open and write on the planinventory 
  file = SPIFFS.open("/plantInventory.json", "w");

  //if the file exist write the values 
  if (file) {
    serializeJsonPretty(doc, file);

    //close the file 
    file.close();

  } else {
    Serial.println("Error writing on the file.");
  }
}
//delete the plant by id 
void deletePlantByID(int idToDelete) {
  
  // open and read the file 
  File file = SPIFFS.open("/plantInventory.json", "r");

  //check if the file exist 
  if (!file || file.isDirectory()) {
    Serial.println("Fichier introuvable ");
    return;
  }

  // Create a JSON document with a capacity of 1024 byte
  StaticJsonDocument<1024> doc;
  DeserializationError error = deserializeJson(doc, file);

  //close file
  file.close();

  //checks if the array has a problem 
  if (error || !doc.is<JsonArray>()) {
    Serial.println("Erreur JSON");
    return;
  }
  JsonArray array = doc.as<JsonArray>();

  // delete the plant with by the id 
  for (JsonArray::iterator it = array.begin(); it != array.end(); ++it) {
    if ((*it)["ID"] == idToDelete) {
      array.remove(it);
      break;
    }
  }

  // write the new array on the file json 
  file = SPIFFS.open("/plantInventory.json", "w");
  if (!file) {
    Serial.println("Erreur d’écriture");
    return;
  }
  serializeJsonPretty(array, file);

  //close file
  file.close();

  // update the variables 
  if (idToDelete == 1) plant1 = false;
  if (idToDelete == 2) plant2 = false;


}

//read and display the inventory for plant 1
void readInventoryPlant1(WiFiClient& client) {

  //open the inventory JSON file in read-only mode
  File file = SPIFFS.open("/plantInventory.json", FILE_READ);

  // If the file doesn't exist or is a directory, show error message to client
  if (!file || file.isDirectory()) {
    client.print("<p>Fichier introuvable</p>");
    return;
  }

  // Allocate a JSON document capable of holding up to 1024 bytes
  StaticJsonDocument<1024> doc;
  DeserializationError error = deserializeJson(doc, file);

  //close the file
  file.close();

  //check if it can reads the json file 
  if (error) {
    client.print("<p>Erreur de lecture JSON</p>");
    return;
  }

  //convert  the root of the document into a JSON array
  JsonArray array = doc.as<JsonArray>();

  // Iterate over each object in the array
  for (JsonObject plant : array) {

    //checks the id into the array 
    if (plant["ID"] == 1) {

      //converts the type and the value 
      const char* type = plant["type"];
      int percent = plant["percent"];
      
      levelWateringPlant1 = percent;
      typePlant1=type;


      // open a button div for the plant 
      client.println("<div id=\"plant-1\">");

      //modify button 
      client.println("<form action=\"/modifyPlant1\" method=\"POST\">");  
      client.println("<label>Plante:</label>");
      //forms plant 
      client.print  ("<input name=\"type\"     value=\""); client.print(type);     client.println("\" required>");
      client.println("<label>% arrosage:</label>");
      client.print  ("<input name=\"percent\" type=\"number\" value=\""); client.print(percent); client.println("\" required>");

      //modify button 
      client.println("<input type=\"submit\" value=\"Modifier\" style=\"padding:6px 16px;\">");
      client.println("</form>");

      //delete button 
      client.println("<p><a href=\"/deletePlant?id=1\">");
      client.println("  <button style=\"padding:6px 16px;\">Supprimer</button>");
      client.println("</a></p>");

      client.println("</div>");
      break;
    }
  }
}
//read and display the inventory for plant 2
void readInventoryPlant2(WiFiClient& client) {

  //open the inventory JSON file in read-only mode
  File file = SPIFFS.open("/plantInventory.json", FILE_READ);

  // If the file doesn't exist or is a directory, show error message to client
  if (!file || file.isDirectory()) {
    client.print("<p>Fichier introuvable</p>");
    return;
  }

  //allocate a JSON document capable of holding up to 1024 bytes
  StaticJsonDocument<1024> doc;
  DeserializationError error = deserializeJson(doc, file);
  
  //close the file 
  file.close();

  //check if it can reads the json file
  if (error) {
    client.print("<p>Erreur de lecture JSON</p>");
    return;
  }

  
  //convert  the root of the document into a JSON array
  JsonArray array = doc.as<JsonArray>();
  
  // Iterate over each object in the array
  for (JsonObject plant : array) {
    
    //checks the id into the array 
    if (plant["ID"] == 2) {

      //converts the type and the value
      const char* type = plant["type"];
      int percent = plant["percent"];

      levelWateringPlant2 = percent;
      typePlant2 =type;


      // open a button div for the plant 
      client.println("<div id=\"plant-2\">");
      //modify button 
      client.println("<form action=\"/modifyPlant2\" method=\"POST\">");  
      client.println("<label>Plante:</label>");
      
      //forms of the plant 
      client.print  ("<input name=\"type\"     value=\""); client.print(type);     client.println("\" required>");
      client.println("<label>% arrosage:</label>");
      client.print  ("<input name=\"percent\" type=\"number\" value=\""); client.print(percent); client.println("\" required>");

      //modify button 
      client.println("<input type=\"submit\" value=\"Modifier\" style=\"padding:6px 16px;\">");
      client.println("</form>");

      //button delete 
      client.println("<p><a href=\"/deletePlant?id=2\">");
      client.println("  <button style=\"padding:6px 16px;\">Supprimer</button>");
      client.println("</a></p>");

      client.println("</div>");
      break;
    }
  }
}

 

