#include <SPI.h>
#include <SoftwareSerial.h>

// Sensors pins

#define S1_TX_Pin 16
#define S2_TX_Pin 16
#define S3_TX_Pin 16
#define S4_TX_Pin 16

#define BUZZER_VCC_Pin 4
#define BUZZER_IO_Pin 5

#define S1_RX_Pin 6
#define S1_VCC_Pin 7

#define S2_RX_Pin 8
#define S2_VCC_Pin 9

// TODO set other ports as for 4!
#define S3_RX_Pin 14
#define S3_VCC_Pin 15


// led pin (13) is left out
#define S4_RX_Pin 14
#define S4_VCC_Pin 15

SoftwareSerial sensor1(S1_RX_Pin, S1_TX_Pin);
SoftwareSerial sensor2(S2_RX_Pin, S2_TX_Pin);
// S3 pins TODO (MOSI/MISO pins are needed for SPI)
SoftwareSerial sensor3(S4_RX_Pin, S4_TX_Pin);
SoftwareSerial sensor4(S4_RX_Pin, S4_TX_Pin);

// print measured  values to seriraloutput in cm
boolean printSerial = false;

boolean measuring = true;

// last measured values in cm (0xFF sensor N/A or not  working)
byte lastMeasure[] = {0, 0, 0, 0};
const byte sensorOffline = 0xFF;
 
//temp variables for measure function
byte startByte = 0, mmLow = 0, mmHigh = 0, sumByte = 0;
unsigned int chkSum = 0; // calculated checksum
unsigned int mm = 0; //measured mm.


void setup() {
  if (printSerial) {
    Serial.begin(9600);
    Serial.println("started...");
  }
  pinMode(S1_TX_Pin, OUTPUT);
  pinMode(S2_TX_Pin, OUTPUT);

  pinMode(S1_RX_Pin, INPUT);
  pinMode(S1_VCC_Pin, OUTPUT);

  pinMode(S2_RX_Pin, INPUT);
  pinMode(S2_VCC_Pin, OUTPUT);

pinMode(S3_RX_Pin, INPUT);
pinMode(S3_VCC_Pin, OUTPUT);

  pinMode(S4_RX_Pin, INPUT);
  pinMode(S4_VCC_Pin, OUTPUT);

  // set the data rate for the SoftwareSerial port
  sensor1.begin(9600);
  sensor2.begin(9600);
  sensor3.begin(9600);
  sensor4.begin(9600);

  delay(1000); // wait 1 sec - for new version upload!

  digitalWrite(S1_VCC_Pin, LOW);
  digitalWrite(S2_VCC_Pin, LOW);
  digitalWrite(S3_VCC_Pin, LOW);
  digitalWrite(S4_VCC_Pin, LOW);

  lastMeasure[0] = checkSensors(sensor1, S1_VCC_Pin);
  lastMeasure[1] = checkSensors(sensor2, S2_VCC_Pin);
  lastMeasure[2] = checkSensors(sensor3, S3_VCC_Pin);
  lastMeasure[3] = checkSensors(sensor4, S4_VCC_Pin);
  if (printSerial) {
    Serial.println("sensors checked. ");
  }

  pinMode(BUZZER_VCC_Pin, OUTPUT);

  // SPI slave initialization 
  // (arduino is the slave,  RPi is the master)
  
  // have to send on master in, *slave out*
  pinMode(MISO, OUTPUT);
  // turn on SPI in slave mode
  SPCR |= bit(SPE);
  // turn on interrupts
  SPCR |= bit(SPIE);  
}

// SPI interrupt routine (should be minimum possible)
ISR (SPI_STC_vect)
{
  unsigned long c = SPDR; // value from the RPI (32 bit!)

  // if value is 0xFFFF_FFFF -> send all measured data!


  // if the value is between 0x00001 .. 0x00004
  // then return the measured distance for that sensor.
  if (c == 0xA0) {
   // command used to switch debug (serial print on/off)
    printSerial = !printSerial;
    SPDR = 0xAB;
  } else  if (c == 0xA1) {
   // command used to switch measuring on and off
    measuring = !measuring;
    SPDR = 0xAC;
  } else if (c >= 1 && c <= 4)  { // sensors 1 -4
       // c == 1 is the first sensor!
      byte sc = c-1; 
      if (lastMeasure[sc] == sensorOffline) {
        SPDR = 0xFA;
      }else {
        SPDR = 0xFF & lastMeasure[sc];
      }
  } else {
    // the recieved command is not between 1 and nr of sensors
    // 1 -> first sensor! (not starting with 0
     SPDR = 0xAA;
  }
}

void loop() {

  unsigned int s1cm = 0;
  unsigned int mincm = 100;
  if (measuring) {
    for (byte sc = 0; sc <= 3; sc++) {
      if (lastMeasure[sc] != -1) {
        if ( sc == 0) {
          s1cm = measure(sensor1, S1_VCC_Pin);
        } else if ( sc == 1) {
          s1cm = measure(sensor2, S2_VCC_Pin);
        } else if ( sc == 2) {
          s1cm = measure(sensor3, S3_VCC_Pin);
        } else if ( sc == 3) {
          s1cm = measure(sensor4, S4_VCC_Pin);
        }
        if (s1cm != lastMeasure[sc]) {
          lastMeasure[sc] = s1cm;
        }
        // delay(50);
        if (s1cm < mincm) {
          mincm = s1cm;
        }
      }
    }
  }else {
    lastMeasure[0] = 0;
    lastMeasure[1] = 0;
    lastMeasure[2] = 0;
    lastMeasure[3] = 0;
  }
  delay(1000);
  if (printSerial) {
    // print measurements result
    for (byte sc = 0; sc <= 3; sc++) {
      Serial.print(lastMeasure[sc]);
      Serial.print(" ");
    }
    Serial.println(" ");
  }
/*
  unsigned long v4 = (unsigned long) lastMeasure[3];
  unsigned long v3 = (unsigned long) lastMeasure[2] << 8;
  unsigned long v2 = (unsigned long) lastMeasure[1] << 16;
  unsigned long v1 = (unsigned long) lastMeasure[0] << 24;
  measured = v1 | v2 | v3 | v4;
*/

}

// check if the sensor is not returning a 0xFF within one second deactivate sensor
byte checkSensors(SoftwareSerial &sensor, byte vccPin) {
  digitalWrite(vccPin, HIGH);
  sensor.listen();
  byte count = 0;
  byte startByte = 0;
  do {
    startByte = sensor.read();
    delay(100);
    count++;
  } while ( startByte != 0xFF && count < 10);
  digitalWrite(vccPin, LOW);

  if (count >= 10) {
    return sensorOffline;
  }
  return 0;
}

byte measure(SoftwareSerial &sensor, byte vccPin) {
  digitalWrite(vccPin, HIGH);

  sensor.listen();
  // todo eventually 3 measurements with avg?
  delay(50);
  do {
    byte count = 0;
    do {
      startByte = sensor.read();
      count++;
    } while ( startByte != 0xFF && count <= 10);
    if (count >= 10) {
      digitalWrite(vccPin, LOW);
      return 1;
    }
    // wait till the next 3 bytes are available
    while (sensor.available() < 3) {
      // approx needed for the next 3 bytes
      // delayMicroseconds(10);
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


