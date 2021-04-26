/*
  Blink + SPI receive mit interrupt
*/

#include <SPI.h>
#include <SoftwareSerial.h>

#define D2 2 // LED_Pin

#define S1_TX_Pin 16
#define S2_TX_Pin 16
#define S3_TX_Pin 16
#define S4_TX_Pin 16

#define S1_RX_Pin 8
#define S1_VCC_Pin 9

#define S2_RX_Pin 6
#define S2_VCC_Pin 7

#define S3_RX_Pin 4
#define S3_VCC_Pin 5

#define S4_RX_Pin 14
#define S4_VCC_Pin 15


#define HELLO_SPI_BYTE 0xAB; // first byte send to SPI after reset

SoftwareSerial sensor1(S1_RX_Pin, S1_TX_Pin);
SoftwareSerial sensor2(S2_RX_Pin, S2_TX_Pin);
SoftwareSerial sensor3(S3_RX_Pin, S3_TX_Pin);
SoftwareSerial sensor4(S4_RX_Pin, S4_TX_Pin);

byte recBuf[4] = {0, 0, 0, 0}; // received byte values are not used 

// last measured values in cm (default werte falls sensor offline)
byte lastMeasure[] = {0xFB, 0xFC, 0xFD, 0xFE};
bool sensorOnline[] = {true, true, true, true};

//temp variables for measure function
byte count = 0,startByte = 0, mmLow = 0, mmHigh = 0, sumByte = 0;
unsigned int chkSum = 0; // calculated checksum
unsigned int mm = 0; //measured mm.

// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pin 2 as an output for LED.
  pinMode(D2, OUTPUT);
  digitalWrite(D2, LOW);

  pinMode(S1_TX_Pin, OUTPUT);
  pinMode(S2_TX_Pin, OUTPUT);
  pinMode(S3_TX_Pin, OUTPUT);
  pinMode(S4_TX_Pin, OUTPUT);

  pinMode(S1_RX_Pin, INPUT);
  pinMode(S1_VCC_Pin, OUTPUT);
  pinMode(S2_RX_Pin, INPUT);
  pinMode(S2_VCC_Pin, OUTPUT);
  pinMode(S3_RX_Pin, INPUT);
  pinMode(S3_VCC_Pin, OUTPUT);
  pinMode(S4_RX_Pin, INPUT);
  pinMode(S4_VCC_Pin, OUTPUT);

  digitalWrite(S1_VCC_Pin, LOW);
  digitalWrite(S2_VCC_Pin, LOW);
  digitalWrite(S3_VCC_Pin, LOW);
  digitalWrite(S4_VCC_Pin, LOW);

  // initialize softserial für JSN SRN04T
  sensor1.begin(9600);
  sensor2.begin(9600);
  sensor3.begin(9600);
  sensor4.begin(9600);

  digitalWrite(D2, HIGH);
  checkSensors(0, sensor1, S1_VCC_Pin);
  checkSensors(1, sensor2, S2_VCC_Pin);
  checkSensors(2, sensor3, S3_VCC_Pin);
  checkSensors(3, sensor4, S4_VCC_Pin);
  digitalWrite(D2, LOW);

  for (byte i = 1; i<=3; i++) {
    delay(100);
    digitalWrite(D2, HIGH);
    delay(100);
    digitalWrite(D2, LOW);
  }
  
  // SPI INITIALISATION
  pinMode(MISO, OUTPUT);

  /* Enable SPI */
  SPCR = (1<<SPE);
  SPDR = HELLO_SPI_BYTE;

}

// the loop function runs over and over again forever
void loop() {
   digitalWrite(D2, HIGH);
   if (sensorOnline[0]) {
    lastMeasure[0] = measure(sensor1, S1_VCC_Pin);
   }
   if (sensorOnline[1]) {
    lastMeasure[1] = measure(sensor2, S2_VCC_Pin);
   }      
   if (sensorOnline[2]) {
    lastMeasure[2] = measure(sensor3, S3_VCC_Pin);
   }      
   if (sensorOnline[3]) {
    lastMeasure[3] = measure(sensor4, S4_VCC_Pin);
   }
   digitalWrite(D2, LOW);
   
   noInterrupts();
   spi4Bytes();
   interrupts();
 }

// check if the sensor is not returning a 0xFF within one second deactivate sensor
void checkSensors(byte sensorIndex, SoftwareSerial &sensor, byte vccPin) {
  digitalWrite(vccPin, HIGH);
  sensor.listen();
  count = 0;
  startByte = 0;
  do {
    if (sensor.available() > 0) {
      startByte = sensor.read();
    }
    delay(100);
    count = count + 1;
  } while ( startByte != 0xFF && count < 30);
  digitalWrite(vccPin, LOW);

  if (count >= 30) {
    sensorOnline[sensorIndex] = false;
  }else {
    sensorOnline[sensorIndex] = true;
  }
}

byte measure(SoftwareSerial &sensor, byte vccPin) {
  digitalWrite(vccPin, HIGH);
  sensor.listen(); 
  delay(10);
  do {
    // wait till the next 3 bytes are available
    while (sensor.available() <= 4) {
      // approx needed for the next 3 bytes
      // delayMicroseconds(10);
    }

    startByte = sensor.read();
    if (startByte != 0xFF) {
      while (sensor.overflow()) {}
      continue;
    }
    mmHigh = sensor.read();
    mmLow = sensor.read();
    sumByte = sensor.read();

    chkSum =  0xFF + mmHigh + mmLow;
    if (lowByte(chkSum) == sumByte) {
      mm = (mmHigh << 8) + mmLow;
    } else {
      mm = 0;
    }
  } while (mm == 0);

  digitalWrite(vccPin, LOW);

  return mm / 10;
}


 void spi4Bytes() { 
   byte valFromSPIMaster = 0;
   byte bCounter = 0;
   /* Wait till a four byte EXCHANGE is done */
   while (bCounter <= 3)  {
     while(!(SPSR & (1<<SPIF)))
     ;
     recBuf[bCounter] = SPDR;
     SPDR = lastMeasure[bCounter++];
   }
 }
 
