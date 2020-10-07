// Arduino sketch to communicate with a Honeywell 32322550 air quality sensor
// For info on the sensor see https://sensing.honeywell.com/honeywell-sensing-particulate-hpm-series-datasheet-32322550.pdf

// The sensor automatically sends PM2.5 and PM10 data about every second so this code only receives the data and never
// transmits to the sensor. One benefit to this is that the sensor can only handle 3.3v serial IO while the Arduino Uno I'm using
// has 5V IO. Since I don't need the Uno to transmit to the sensor, I can avoid using a level-shifter.

// There are lots of serial log messages but the Uno doesn't need to be connected to a computer. The setup can be powered just
// from the USB "B" connector plugged into a power-block, USB battery, etc. The Uno provides power to the sensor.

// Oct 2020

// Issues:
// Every few reads, the code detects a bad checksum - not sure why. This is hardly noticeable since it will update again
// in a few seconds.


#include <NeoSWSerial.h>

#include <TimerOne.h>
#include <Wire.h>
#include <MultiFuncShield.h>
#include <stdlib.h>
#include <stdio.h>

/////////////////////////////////////////////////////////////////
  
float aqi25Table[6][4] = {
  {0,     12,    0,   50},  // Good
  {12.1,  35.4,  51,  100}, // Moderate
  {35.5,  55.4,  101, 150}, // Unhealthy for sensitive people
  {55.5,  150.4, 151, 200}, // Unhealthy
  {150.5, 250.4, 201, 300}, // Unhealthy - Very
  {250.5, 500.4, 301, 500}  // Hazardous
};


byte readVal[32];
int n, i, suma, aqi;

long PM25;
long PM10;

#define rxPin 5
#define txPin 6


// Instantiate the serial port that connects to the sensor
NeoSWSerial altSerial(rxPin, txPin); // RX, TX

// rawToAqi converts a raw25 number to the AQI value
int rawToAqi(int raw)
{
  int myAqi;
  for (i=0; i < 6; i++)
    {
      if (raw <= aqi25Table[i][1])
	{
            myAqi = ((aqi25Table[i][3]-aqi25Table[i][2])/(aqi25Table[i][1]-aqi25Table[i][0]))*(raw-aqi25Table[i][0])+aqi25Table[i][2]+0.5;
            break;
	}
    }
    return myAqi;
}


void setup() {
  Timer1.initialize();
  MFS.initialize(&Timer1); // initialize multi-function shield library

  MFS.write("");
  MFS.write("AQI");
  delay(1000);

  pinMode(rxPin, INPUT);        // sets the digital pin rxPin as input
  pinMode(txPin, OUTPUT);       // sets the digital pin txPin as output

  Serial.begin(9600);
//  while (!Serial);           // Disable this since we don't always have the serial port to a computer connected
  altSerial.begin(9600);
  delay(100);

}

// Main loop...
void loop()
{
  n = 0;
  readVal[n] = 0;
  // Data comes about every second. Since we've been waiting, the buffer should have accumulated data that should be removed
  while (altSerial.available()>0) {altSerial.read();} // Drain/clear receive buffer

  Serial.print("Note: waiting for data to arrive... ");
  while (altSerial.available()==0) {} // Spin waiting for data

  // The first two bytes of the stream will be 66 and 77. Watch for those...
  
  Serial.print("looking for start byte (66)... ");
  while (readVal[n] != 66) { readVal[n] = altSerial.read();} // Spin waiting for the 66
  
  suma = readVal[0];
  Serial.print("reading remaining 31 bytes... ");
  for(n=1; n<32; n++)
  {
    while (altSerial.available()==0) {} // Spin waiting for data
    readVal[n] = altSerial.read();
    if (n < 30)  { suma = suma + readVal[n]; } // prepare for checksum
    //    Serial.print("Note: read byte: ");
    //    Serial.println(readVal[n]);
  }
  
  Serial.println("got all 32 bytes");
  if ((readVal[1] == 77) && (suma == (readVal[30]*256+readVal[31])))
    {
      PM25 = readVal[6]*256+readVal[7]; // See Honeywell datasheet for info
      aqi = rawToAqi(PM25);
      MFS.write("");
      MFS.write(aqi);
      PM10 = readVal[8]*256+readVal[9]; // See Honeywell datasheet for info
      //      Serial.print("Note: raw PM25 and PM10 are:");  
      //      Serial.print(PM25);  
      //      Serial.print(", ");  
      //      Serial.println(PM10);  
    }
  else  // Something went wrong
      {
        if (readVal[1] != 77)
          {
	    Serial.print("Error: second byte was bad - not 77: ");
	    Serial.println(readVal[1]);
          }
	else
	  {
	    Serial.print("Error: checksum was bad; ");
	    Serial.print("predicted checksum is: ");
	    Serial.print(readVal[30]*256+readVal[31]);
	    Serial.print("; summed bytes for checksum is: ");
	    Serial.println(suma);
	  }
      }
  delay(3000); // Just update about every 3 seconds
}
