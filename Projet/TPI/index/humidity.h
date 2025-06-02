#ifndef HUMIDITY_H
#define HUMIDITY_H

const int airValue = 4095;     // Value of the dry sensor calculated 
const int waterValue = 1200;   // Value of the wet sensor calculated 


int moisturePercent1;       // Moisture level for plant 1 form 0 to 100%
int moisturePercent2;       // Moisture level for plant 2 form 0 to 100%

constexpr uint8_t SensorPin1 = 36;          // pin GPIO36 for taking the data of the sensor humidity 1
constexpr uint8_t SensorPin2 = 34;          // pin GPIO34 for taking the data of the sensor humidity 2



// Read soil moisture value and convert to percentage for the plant 1 
void moistureSensor1(){

  int raw1 = analogRead(SensorPin1);  //Reds the sensor pin 
  
  
  moisturePercent1 = map(raw1, waterValue, airValue, 100, 0); // Map and clamp result between 0 and 100
  moisturePercent1 = constrain(moisturePercent1, 0, 100);     //give the percent of the humidity between 0 and 100% 
}

// Read soil moisture value and convert to percentage for the plant 2
void moistureSensor2(){
  int raw2 = analogRead(SensorPin2); //Reds the sensor pin 

  moisturePercent2 = map(raw2, waterValue, airValue, 100, 0); // Map and clamp result between 0 and 100
  moisturePercent2 = constrain(moisturePercent2, 0, 100);     //give the percent of the humidity between 0 and 100% 
}



#endif