#include <Adafruit_Si7021.h>
#include <SparkFunCCS811.h>
#include <SparkFunMPL3115A2.h>

#include <SPI.h>
#include <SD.h>

#define CCS811_ADDR 0x5B //Default I2C Address
#define THRESHOLD 60000

//Prepare I2C sensors
Adafruit_Si7021 si7021 = Adafruit_Si7021();
CCS811 ccs811(CCS811_ADDR);
MPL3115A2 mpl3115A2;

//Prepare analog variables
int soilSensorPin = A6;

//Prepare SD writer
const int chipSelect = 10;

//initialize timer variables
unsigned long lastTime = 0;
unsigned int minuteCounter = 0;

void setup() {
  /*Serial.begin(9600);  // Start serial for output

  // wait for serial port to open
  while (!Serial) {
    delay(10);
  }
  
  if (!si7021.begin()) {
    Serial.println("Did not find Si7021 sensor!");
    while (true);
  }
  */
  si7021.begin();
  mpl3115A2.begin(); // Get sensor online
  
    // Configure MPL3115A2  
  mpl3115A2.setOversampleRate(7); // Set Oversample to the recommended 128
  mpl3115A2.enableEventFlags(); // Enable all three pressure and temp event flags
  mpl3115A2.setModeBarometer(); // Measure pressure in Pascals from 20 to 110 kPa
  //mpl3115A2.setModeAltimeter();
  
  ccs811.begin();

  //Serial.print("Initializing SD card...");

  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    //Serial.println("Card failed, or not present");
    // don't do anything more:
    while (1);
  }
  //Serial.println("card initialized.");

  lastTime = millis();
  
  writeHeaderToCard();
}

void loop() {
  unsigned long thisTime = millis();
  if(thisTime - lastTime >= THRESHOLD) {
    lastTime = thisTime;
  
    //Check to see if data is available
    if (ccs811.dataAvailable())
    {
      //Calling this function updates the global tVOC and eCO2 variables
      ccs811.readAlgorithmResults();
      //printInfoSerial fetches the values of tVOC and eCO2
      //printCCS811Serial();
      //printSI7021Serial();
      //printMPL3115A2Serial();
      writeDataToCard(minuteCounter);

      float SItempC = si7021.readTemperature();
      float SIhumid = si7021.readHumidity();
    
      //This sends the temperature data to the CCS811
      ccs811.setEnvironmentalData(SIhumid, SItempC);

      minuteCounter += 1;
    }
    else if (ccs811.checkForStatusError())
    {
      //If the CCS811 found an internal error, print it.
      //printSensorError();
    }

  //delay(59869);
  //delay(4869);
  }
}

//---------------------------------------------------------------
/*void printCCS811Serial()
{

  //getCO2() gets the previously read data from the library
  Serial.println("CCS811 data:");
  Serial.print(" CO2 concentration : ");
  Serial.print(ccs811.getCO2());
  Serial.println(" ppm");

  //getTVOC() gets the previously read data from the library
  Serial.print(" TVOC concentration : ");
  Serial.print(ccs811.getTVOC());
  Serial.println(" ppb");
  Serial.println();
}

void printSI7021Serial() {
  Serial.println("Si7021 data:");
  Serial.print(" Temperature: ");
  Serial.print(si7021.readTemperature(), 2);
  Serial.println(" degrees C");

  Serial.print(" %RH: ");
  Serial.print(si7021.readHumidity(), 2);
  Serial.println(" %");
  Serial.println();
}

void printMPL3115A2Serial() {
  Serial.println("MPL3115A2 data:");
  Serial.print(" Pressure: ");
  Serial.print(mpl3115A2.readPressure(), 2);
  Serial.println(" Pa");

  Serial.print(" Temperature: ");
  Serial.print(mpl3115A2.readTemp(), 2);
  Serial.println(" degrees C");

  Serial.println();
  Serial.println();
}*/

void writeHeaderToCard() {
  String headerString = "";

  headerString += "elapsedTime";
  headerString += ";";
  headerString += "ccs811C02";
  headerString += ";";
  headerString += "ccs811TVOC";
  headerString += ";";
  headerString += "si7021Temperature";
  headerString += ";";
  headerString += "si7021Humidity";
  headerString += ";";
  headerString += "mpl3115A2Pressure";
  headerString += ";";
  headerString += "mpl3115A2Temperature";
  headerString += ";";
  headerString += "soilMoisture";

  File dataFile = SD.open("datalog.txt", FILE_WRITE);

  // if the file is available, write to it:
  if (dataFile) {
    dataFile.println(headerString);
    dataFile.close();
  }
  // if the file isn't open, pop up an error:
  else {
  //  Serial.println("error opening datalog.txt to write header");
  }
}

void writeDataToCard(int minCount) {
  // make a string for assembling the data to log:
  String dataString = "";
  long dataTime;
  dataTime = millis();

  dataString += String(dataTime);
  dataString += ";";
  if (minCount <= 20){
    dataString = dataString + ccs811.getCO2() + "*";
    dataString += ";";
    dataString = dataString + ccs811.getTVOC() + "*";
    dataString += ";";
  }
  else {
    dataString += ccs811.getCO2();
    dataString += ";";
    dataString += ccs811.getTVOC();
    dataString += ";";
  }
  dataString += si7021.readTemperature();
  dataString += ";";
  dataString += si7021.readHumidity();
  dataString += ";";
  dataString += mpl3115A2.readPressure();
  dataString += ";";
  dataString += mpl3115A2.readTemp();
  dataString += ";";
  dataString += analogRead(soilSensorPin);

  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  File dataFile = SD.open("datalog.txt", FILE_WRITE);

  // if the file is available, write to it:
  if (dataFile) {
    dataFile.println(dataString);
    dataFile.close();
    // print to the serial port too:
  //  Serial.println(dataString);
  }
  // if the file isn't open, pop up an error:
  else {
  //  Serial.println("error opening datalog.txt to write data");
  }
}

//printDriverError decodes the CCS811Core::status type and prints the
//type of error to the serial terminal.
//
//Save the return value of any function of type CCS811Core::status, then pass
//to this function to see what the output was.
/*void printDriverError( CCS811Core::status errorCode )
{
  switch ( errorCode )
  {
    case CCS811Core::SENSOR_SUCCESS:
      Serial.print("SUCCESS");
      break;
    case CCS811Core::SENSOR_ID_ERROR:
      Serial.print("ID_ERROR");
      break;
    case CCS811Core::SENSOR_I2C_ERROR:
      Serial.print("I2C_ERROR");
      break;
    case CCS811Core::SENSOR_INTERNAL_ERROR:
      Serial.print("INTERNAL_ERROR");
      break;
    case CCS811Core::SENSOR_GENERIC_ERROR:
      Serial.print("GENERIC_ERROR");
      break;
    default:
      Serial.print("Unspecified error.");
  }
}
*/

//printSensorError gets, clears, then prints the errors
//saved within the error register.
/*void printSensorError()
{
  uint8_t error = ccs811.getErrorRegister();

  if ( error == 0xFF ) //comm error
  {
    //Serial.println("Failed to get ERROR_ID register.");
  }
  else
  {
    Serial.print("Error: ");
    if (error & 1 << 5) Serial.print("HeaterSupply");
    if (error & 1 << 4) Serial.print("HeaterFault");
    if (error & 1 << 3) Serial.print("MaxResistance");
    if (error & 1 << 2) Serial.print("MeasModeInvalid");
    if (error & 1 << 1) Serial.print("ReadRegInvalid");
    if (error & 1 << 0) Serial.print("MsgInvalid");
    Serial.println();
  }
}*/
