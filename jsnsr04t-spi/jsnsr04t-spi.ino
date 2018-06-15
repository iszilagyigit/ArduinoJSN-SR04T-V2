#include <SPI.h>

const byte sensorOffline = 0xFF;

// last measured values in cm (-1 sensor N/A or not  working)
byte lastMeasure[] = {23, 27, sensorOffline, 29};
// 32 bit having the 4 bytes from lastMeasure
volatile unsigned long measured;

void setup() {
 
  // put your setup code here, to run once:
  pinMode(A3,OUTPUT);
  digitalWrite(A3, HIGH); //led off

   // have to send on master in, *slave out*
  pinMode(MISO, OUTPUT);

  // turn on SPI in slave mode
  SPCR |= bit(SPE);

  // turn on interrupts
  SPCR |= bit(SPIE);

  unsigned long v4 = (unsigned long) lastMeasure[3];
  unsigned long v3 = (unsigned long) lastMeasure[2] << 8;
  unsigned long v2 = (unsigned long) lastMeasure[1] << 16;
  unsigned long v1 = (unsigned long) lastMeasure[0] << 24;
  measured = v1 | v2 | v3 | v4;

}

// SPI interrupt routine (should be minimum possible)
ISR (SPI_STC_vect)
{
  unsigned long c = SPDR; // value from the RPI (32 bit!)

  // if value is 0xFFFF_FFFF -> send all measured data!
  // if the value is between 0x00001 .. 0x00004
  // then return the measured distance for that sensor.
  if (c == 0xFFFFFFFFL) {
    SPDR = measured;
  }else if (c >= 1 && c <= 4)  { // sensors 1 -4
    if (lastMeasure[c-1] == sensorOffline) {
      SPDR = 0xFAL;
    }else {
      SPDR = 0x000000FFL & lastMeasure[c-1];
    }
  }
}


void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(A3, LOW); //led on
  delay(1000);
  digitalWrite(A3, HIGH); //led off
  delay(1000);
}
