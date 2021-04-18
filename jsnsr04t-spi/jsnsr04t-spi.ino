#include <SPI.h>

const byte sensorOffline = 0xFF;

// last measured values in cm (-1 sensor N/A or not  working)
byte lastMeasure[] = {0x19, 0x2A, sensorOffline, 0x2C};
// 32 bit having the 4 bytes from lastMeasure
volatile unsigned long measured;

void setup() {
 
  // put your setup code here, to run once:
  pinMode(LED_BUILTIN,OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH); //led off

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
  measured = 0x192AFF2D;

}

// SPI interrupt routine (should be minimum possible)
ISR (SPI_STC_vect)
{
  unsigned long c = SPDR; // value from the RPI (32 bit!)

  // if value is 0xFFFF_FFFF -> send all measured data!
  // if the value is between 0x00001 .. 0x00004
  // then return the measured distance for that sensor.
  if (c == 0xFF) {
    SPDR = measured;
  }else if (c >= 1 && c <= 4)  { // sensors 1 -4
      // c == 1 is the first sensor!
    if (lastMeasure[c-1] == sensorOffline) {
      SPDR = 0xFA;
    }else {
      SPDR = 0xFF & lastMeasure[c-1];
    }
  } else {
    // the recieved command is not between 1 and nr of sensors
    // 1 -> first sensor! (not starting with 0
     SPDR = 0xAA;
  }
}


void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(LED_BUILTIN, LOW); //led on
  delay(1000);
  digitalWrite(LED_BUILTIN, HIGH); //led off
  delay(1000);
}
