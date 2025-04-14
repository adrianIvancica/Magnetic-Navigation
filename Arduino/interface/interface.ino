#include <Arduino.h>
#include <Wire.h>
#include <QMC5883LCompass.h>
#include <TinyGPS++.h>
#include <SoftwareSerial.h>

/*
   It requires the use of SoftwareSerial, and assumes that you have a
   4800-baud serial GPS device hooked up on pins 0(rx) and 1(tx).
*/
static const int TXPin = 1, RXPin = 0;
static const uint32_t GPSBaud = 9600;

// The TinyGPS++ object
TinyGPSPlus gps;

// The serial connection to the GPS device
SoftwareSerial ss(RXPin, TXPin);

// Create a compass
QMC5883LCompass compass;

// Compass Config
// Mode Control (MODE)
byte standby = 0x00;
byte continuous = 0x01;

// Output Data Rate (ODR)
byte HZ10 = 0x00; // 10 HZ
byte HZ50 = 0x04; // 50 HZ
byte HZ100 = 0x08; // 100 HZ
byte HZ200 = 0x0C; // 200 HZ

// Full Scale (RNG)
byte G2 = 0x00; // +/- 2 Gauss range
byte G8 = 0x10; // +/- 8 Gauss range

// Over Sample Ratio (OSR)
byte r64 = 0xC0;
byte r128 = 0x80;
byte r256 = 0x40;
byte r512 = 0x00;

void setup()
{
  Serial.begin(9600);
  Wire.begin();
  ss.begin(GPSBaud);
  compass.init();
  compass.setMode(continuous, HZ200, G8, r512);
  // Redo calibration in new location
  compass.setCalibrationOffsets(146.00, -111.00, -632.00);
  compass.setCalibrationScales(0.90, 0.89, 1.32);
  // Print header row
  Serial.println(F("X,Y,Z,Azimuth,Bearing,Direction,Latitude,Longitude,Altitude,Date,Time"));
}

static void smartDelay(unsigned long ms)
{
  unsigned long start = millis();
  do 
  {
    while (ss.available())
      gps.encode(ss.read());
  } while (millis() - start < ms);
}

static void printInt(unsigned long val, bool valid, int len)
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

static void printFloat(float val, bool valid, int len, int prec)
{
  if (!valid)
  {
    while (len-- > 1)
      Serial.print('*');
  }
  else
  {
    Serial.print(val, prec);
    /*
    int vi = abs((int)val);
    int flen = prec + (val < 0.0 ? 2 : 1); // . and -
    flen += vi >= 1000 ? 4 : vi >= 100 ? 3 : vi >= 10 ? 2 : 1;
    for (int i=flen; i<len; ++i)
      Serial.print(' ');
      */
  }
  smartDelay(0);
}

static void printDateTime(TinyGPSDate &d, TinyGPSTime &t)
{
  if (!d.isValid())
  {
    Serial.print(F("**********,"));
  }
  else
  {
    char sz[32];
    sprintf(sz, "%02d/%02d/%02d,", d.month(), d.day(), d.year());
    Serial.print(sz);
  }
  
  if (!t.isValid())
  {
    Serial.print(F("********"));
  }
  else
  {
    char sz[32];
    sprintf(sz, "%02d:%02d:%02d", t.hour(), t.minute(), t.second());
    Serial.print(sz);
  }

  //printInt(d.age(), d.isValid(), 5);
  smartDelay(0);
}

void displayMagInfo(){
  int x, y, z, a, b;
	char myArray[3];

  x = compass.getX();
	y = compass.getY();
	z = compass.getZ();
	
	a = compass.getAzimuth();
	
	b = compass.getBearing(a);

	compass.getDirection(myArray, a);
  
	Serial.print(x);
  Serial.print(F(","));
	Serial.print(y);
  Serial.print(F(","));
	Serial.print(z);
  Serial.print(F(","));
	Serial.print(a);
  Serial.print(F(","));
	Serial.print(b);
  Serial.print(F(","));
  //strcmp returns 0 if strings are the same
  if (myArray[0] != ' ')
	  Serial.print(myArray[0]);
  if (myArray[1] != ' ')
	  Serial.print(myArray[1]);
  if (myArray[2] != ' ')
	  Serial.print(myArray[2]);
}

void displayGPSInfo() {
  Serial.print(F(","));
  printFloat(gps.location.lat(), gps.location.isValid(), 11, 6);
  Serial.print(F(","));
  printFloat(gps.location.lng(), gps.location.isValid(), 12, 6);
  Serial.print(F(","));
  printFloat(gps.altitude.meters(), gps.altitude.isValid(), 7, 2);
  Serial.print(F(","));
  printDateTime(gps.date, gps.time);
  Serial.println();
}

void loop()
  { 
    // This sketch displays information every time a new sentence is correctly encoded.
    while (ss.available()){
      //Serial.println(F("is available"));
      if (gps.encode(ss.read())){
        //Serial.println(F("read"));
        compass.read();
        displayMagInfo();
        displayGPSInfo();
      }
      //else
        //Serial.println(F("not read"));
    }
    //else
     //Serial.println(F("NOT available"));
    //smartDelay(1000);

    if (millis() > 5000 && gps.charsProcessed() < 10)
      Serial.println(F("No GPS data received: check wiring"));
    }
