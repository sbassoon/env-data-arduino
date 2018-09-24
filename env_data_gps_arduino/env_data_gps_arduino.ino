#include <TinyGPS++.h>
#include <SoftwareSerial.h>

#include <Adafruit_Si7021.h>
#include <SparkFunCCS811.h>
#include <SparkFunMPL3115A2.h>

#include <SPI.h>
#include <SD.h>

#define CCS811_ADDR 0x5B //Default I2C Address
#define THRESHOLD 60000

//Prepare GPS variables
static const int RXPin = 4, TXPin = 3;
static const uint32_t GPSBaud = 9600;
static uint16_t counter = 0;
bool GPSLock = false;
String latstring = "";
String longstring = "";

//Prepare analog variables
static const int soilSensorPin = A6;

//Prepare SD writer
static const int chipSelect = 10;
unsigned long randNumber;

//Prepare LED pins
static const int GLEDPin = 6, RLEDPin = 7;

//initialize timer variables
uint16_t minuteCounter = 0;

//Prepare I2C sensors
Adafruit_Si7021 si7021 = Adafruit_Si7021();
CCS811 ccs811(CCS811_ADDR);
MPL3115A2 mpl3115A2;

// The TinyGPS++ object
TinyGPSPlus gps;

// The serial connection to the GPS device
SoftwareSerial ss(RXPin, TXPin);

void setup()
{
  //Serial.begin(115200); // Start serial for output
  ss.begin(GPSBaud);

  pinMode(GLEDPin, OUTPUT);
  pinMode(RLEDPin, OUTPUT);

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
  if (!SD.begin(chipSelect)) { //if no, throw error and do nothing more:
    //Serial.println("Card failed, or not present");
    digitalWrite(RLEDPin, HIGH);
    digitalWrite(GLEDPin, HIGH);
    while (1);
  }

  randomSeed(analogRead(0));
  
  //Serial.println("card initialized.");
  writeHeaderToCard();
}

void loop()
{
  if (GPSLock == false) {
    //Broadcast functioning incoming data
    digitalWrite(GLEDPin, HIGH);
    smartDelay(50);
    digitalWrite(GLEDPin, LOW);
    /*Serial.println(gps.location.lat(), 6);
    Serial.println(gps.location.lng(), 6);
    Serial.println(gps.date.year());
    Serial.println(gps.time.hour());
    Serial.print(counter);
    Serial.print(" ");
    Serial.println(gps.hdop.hdop());*/

    //Broadcast connection and data has been achieved
    if (gps.location.isValid() == true) {
      counter++;
      digitalWrite(RLEDPin, HIGH);
      smartDelay(50);
      digitalWrite(RLEDPin, LOW);
    }
    smartDelay(900);

    //Catch if GPS isn't receiving data
    if (millis() > 5000 && gps.charsProcessed() < 10) {
        while(1) { //throw error and do nothing more:
        digitalWrite(RLEDPin, HIGH);
        delay(250);
        digitalWrite(RLEDPin, LOW);
        delay(50);
        digitalWrite(RLEDPin, HIGH);
        delay(250);
        digitalWrite(RLEDPin, LOW);
        delay(50);
        digitalWrite(GLEDPin, HIGH);
        delay(250);
        digitalWrite(GLEDPin, LOW);
        delay(50);
      }
    }

    if (gps.hdop.hdop() < 1.5 & counter <= 60 & gps.hdop.isValid() == true) {
      setGPSCoordsAndLock();
      printData();
    } else if (gps.hdop.hdop() <= 2.0 & counter > 60 & counter <= 90 & gps.hdop.isValid() == true) {
      setGPSCoordsAndLock();
      printData();
    } else if (gps.hdop.hdop() <= 3.0 & counter <= 120 & counter > 90 & gps.hdop.isValid() == true) {
      setGPSCoordsAndLock();
      printData();
    } else if (gps.hdop.hdop() <= 4.0 & counter <= 150 & counter > 120 & gps.hdop.isValid() == true) {
      setGPSCoordsAndLock();
      printData();
    } else if (gps.hdop.hdop() <= 5.0 & counter <= 180 & counter > 150 & gps.hdop.isValid() == true) {
      setGPSCoordsAndLock();
      printData();
    } else if (gps.hdop.hdop() > 5.0 & counter > 180 & gps.hdop.isValid() == true) {
      setGPSCoordsAndLock();
      printData();
    }

  } else if (GPSLock == true) {
    printData();
  }
}

static void setGPSCoordsAndLock () {     
      GPSLock = true;
      digitalWrite(GLEDPin, HIGH);
     
      latstring = String(float(gps.location.lat()), 6);
      longstring = String(float(gps.location.lng()), 6);
}

//---------------------------------------------------------------

// This custom version of delay() ensures that the gps object
// is being "fed".
static void smartDelay(unsigned long ms)
{
  unsigned long start = millis();
  do
  {
    while (ss.available())
      gps.encode(ss.read());
  } while (millis() - start < ms);
}

void printData() {
  //Check to see if data is available
  if (ccs811.dataAvailable())
  {
    //Calling this function updates the global tVOC and eCO2 variables
    ccs811.readAlgorithmResults();
    //printInfoSerial fetches the values of tVOC and eCO2
    //printFloat(gps.location.lat(), gps.location.isValid(), 11, 6);
    //printFloat(gps.location.lng(), gps.location.isValid(), 12, 6);
    //printDateTime(gps.date, gps.time);
    //printCCS811Serial();
    //printSI7021Serial();
    //printMPL3115A2Serial();
    
    writeDataToCard(minuteCounter, latstring, longstring);

    //This sends the temperature data to the CCS811
    ccs811.setEnvironmentalData(si7021.readHumidity(), si7021.readTemperature());
    
    if(minuteCounter <= 20) {
      minuteCounter += 1;
    }
  }
  smartDelay(THRESHOLD);
}

void writeHeaderToCard() {
  String headerString = "";

  headerString = "d;utc;lat;lng;cC;cTV;sT;sH;mP;mT;Sm";

  File dataFile = SD.open("datalog.txt", FILE_WRITE);

    // if the file is available, write to it:
  if (dataFile) {
    dataFile.println(headerString);
    dataFile.close();
    headerString = "";
  } else {  // if the file isn't open, pop up an error and do nothing else:
    //  Serial.println("error opening datalog.txt to write header");
    digitalWrite(RLEDPin, HIGH);
    while(1);
  }
}

void writeDataToCard(int minCount, String lattistring, String longistring) {
  // make a string for assembling the data to log:
  String dataString = "";

  dataString = gps.date.value();
  dataString += ";";
  dataString += gps.time.hour();
  dataString += ":";
  dataString += gps.time.minute();
  dataString += ":";
  dataString += gps.time.second();
  dataString += ";";
  dataString += lattistring;
  dataString += ";";
  dataString += longistring;
  dataString += ";";
  if (minCount < 20){
    dataString = ccs811.getCO2() + "*";
    dataString += ";";
    dataString = ccs811.getTVOC() + "*";
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
    digitalWrite(GLEDPin, LOW);
    delay(50);
    digitalWrite(GLEDPin, HIGH);
    // print to the serial port too:
    //Serial.println(dataString);
  }
  // if the file isn't open, pop up an error and do nothing else:
  else {
    //Serial.println("error opening datalog.txt to write data");
    digitalWrite(RLEDPin, HIGH);
    while(1);
  }
}



//---------------------------------------------------------------

/*static float writeFloat(float val, bool valid, int len, int prec)
{
  if (!valid)
  {
    while (len-- > 1)
      Serial.print('*');
    Serial.print(' ');
    //return "failed";
  }
  else
  {
    //Serial.print(val, prec);
    int vi = abs((int)val);
    int flen = prec + (val < 0.0 ? 2 : 1); // . and -
    flen += vi >= 1000 ? 4 : vi >= 100 ? 3 : vi >= 10 ? 2 : 1;
    //for (int i = flen; i < len; ++i)
    //  Serial.print(' ');
    return flen;
  }
  smartDelay(0);
}

void printInt(unsigned long val, bool valid, int len)
  {
  char sz[32] = "*****************";
  if (valid)
    sprintf(sz, "%ld", val);
  sz[len] = 0;
  for (int i=strlen(sz); i<len; ++i)
    sz[i] = ' ';
  if (len > 0)
    sz[len-1] = ' ';
  Serial.print(sz);
  smartDelay(0);
  }

static char* writeDateTime(TinyGPSDate &d, TinyGPSTime &t)
{
   char* sz = new char[32];
  
  if (!d.isValid() & !t.isValid()) {
    Serial.print(F("********** "));
    Serial.print(F("******** "));
  }
  else
  {
    sprintf(sz, "%02d/%02d/%02d %02t:%02t:%02t", d.year(), d.month(), d.day(), t.hour(), t.minute(), t.second());
    //String foosz = String(sz);
    
    //char cz[32];
    //sprintf(cz, "%02d:%02d:%02d", timmum.hour(), timmum.minute(), timmum.second());
    //String foocz = String(cz);

    //dPlusT = foosz + ";" + foocz + ";";
    return sz;
  }
  smartDelay(0);
}

void printCCS811Serial()
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
  }

  //printDriverError decodes the CCS811Core::status type and prints the
  //type of error to the serial terminal.
  //
  //Save the return value of any function of type CCS811Core::status, then pass
  //to this function to see what the output was.
  void printDriverError( CCS811Core::status errorCode )
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


  //printSensorError gets, clears, then prints the errors
  //saved within the error register.
  void printSensorError()
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
