/*  Name: Arduino communicationmodule for I2C
 *  Description: Passes values as ADC (Analog Digital Converter) 
 *                via I2C from sensors to RaspberryPi and backwards
 *   
 *  Programmers: A. Koehn, M. Riess, T. Grohe and U. Neumann
 *  Version: 1.2.2 28.02.2022 18:07
 */

//-----Includes:----
#include <Wire.h>  //library for I2C
#include <DHT.h>   //library for DHT-Sensors

//-----Defines:-----
#define DHTTYPE DHT11 //Type of DHT Sensor

//-----Pins:-----
#define DHT_PIN 13 //DHT sensor is on pin 13  
#define LIGHTWRITE_PIN 11  // lightWrite is on pin 11
#define PUMP_PIN 2 //pump gets simulated on Pin 2

DHT dht(DHT_PIN, DHTTYPE);

//-----Variables:-----
//.....Sensor values:.....
byte sensorValues[4] = {0};     //Sensor data is implemented in both ways: as an array for Rx/Tx and as (reference-)variables for using data
byte& air_moisture = sensorValues[0];
byte& brightness = sensorValues[1];
byte& soil_moisture = sensorValues[2];
byte& temperature = sensorValues[3];

//.....Setpoints for regulation:.....
byte lamp_brightness = 0;
byte is_pouring = 0;
byte recieved_value = 0;

//.....Values for writing:.....
int intern_soil_moisture = 1023;

//.....I2C-Communication:.....
int slave_address = 0x0F;

void setup() {
  //-----I2C-----
  Wire.begin(slave_address);
  pinMode(SDA, INPUT); //Makes sure pull-up resistors are off
  pinMode(SCL, INPUT);
  Wire.onReceive(piReceiveHandler);
  Wire.onRequest(piRequestHandler);

  //--------DHT----------
  dht.begin(); //DHT11 Sensor starten

  //---------LED----------
  pinMode(LIGHTWRITE_PIN, OUTPUT);

  //---------Pump---------
  pinMode(PUMP_PIN, OUTPUT);

}

void loop() {
  delay(10000);
  readSensors();
  controlActuators();
}

bool readSensors() {
  humAndTempMeasure();
  lightMeasure();
  soilMoistureMeasure();
  return true; //if reading was successful
}

bool controlActuators() {
  lightControl();
  pumpActuator();
  return true; //if actuating was successful
}

//I2C functions:
void piReceiveHandler(int numOfReceivedBytes) {   //write all new setpoints
  if (numOfReceivedBytes){ 
    recieved_value = Wire.read();
    if (recieved_value == 2){}
    else if (recieved_value > 2){
      lamp_brightness = recieved_value;
    } else {
      is_pouring = recieved_value;
    }  
  }
}

void piRequestHandler() {   //send all sensor data (4 bytes)
  for (int i = 0; i <= 3; i++) {
    Wire.write(sensorValues[i]);
  }
}

//Measuring functions
void humAndTempMeasure() {
  air_moisture = dht.readHumidity(); //die Luftfeuchtigkeit auslesen und unter „Luftfeutchtigkeit“ speichern
  temperature = (dht.readTemperature() + 0.5); //die Temperatur auslesen und unter „Temperatur“ speichern
}

void lightMeasure() {
  brightness = analogRead(A0);
}

void soilMoistureMeasure() {
  intern_soil_moisture = analogRead(A3);
  soil_moisture = intern_soil_moisture >> 2;
}

void lightControl() {
  analogWrite(LIGHTWRITE_PIN, lamp_brightness);
}

void pumpActuator() {
  bool is_poruring_as_bool = is_pouring;
  digitalWrite(PUMP_PIN, is_poruring_as_bool);
}
