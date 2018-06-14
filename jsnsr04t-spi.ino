#include <SPI.h>

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

}

// SPI interrupt routine
ISR (SPI_STC_vect)
{
  digitalWrite(A3, LOW); //led on
  byte c = SPDR;

  if (c <= 4)  { // sensors 1 -4
    SPDR = c+3; //for test
  }
  digitalWrite(A3, HIGH); //led off
} 



void loop() {
  // put your main code here, to run repeatedly:

  delay(1000);

}
