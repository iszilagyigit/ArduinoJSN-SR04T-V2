#include <SoftwareSerial.h>

//first sensors pins

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

#define S3_RX_Pin 10
#define S3_VCC_Pin 11

// led pin (13) is left out

#define S4_RX_Pin 14
#define S4_VCC_Pin 15



SoftwareSerial sensor1(S1_RX_Pin, S1_TX_Pin);
SoftwareSerial sensor2(S2_RX_Pin, S2_TX_Pin);
SoftwareSerial sensor3(S3_RX_Pin, S3_TX_Pin);
SoftwareSerial sensor4(S4_RX_Pin, S4_TX_Pin);

// print measured  values to seriraloutput in cm
boolean printSerial = true;
// make  sound to a buzze
boolean makeSound = true;

// last measured values in cm (-1 sensor N/A or not  working)
int lastMeasure[] = {0, 0, 0, 0};

//temp variables for measure function
byte startByte = 0, mmLow = 0, mmHigh = 0, sumByte = 0;
unsigned int chkSum = 0; // calculated checksum
unsigned int mm = 0; //measured mm.

//sound frequencies
const unsigned int freq[] = {500, 800, 1300};


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
  /*

    digitalWrite(BUZZER_VCC_Pin, HIGH);
  */
}


void loop() {

  unsigned int s1cm = 0;
  unsigned int mincm = 100;

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

  if (makeSound) {
    soundSignal(mincm);
  }

  if (printSerial) {
    // print measurements result
    for (byte sc = 0; sc <= 3; sc++) {
      Serial.print(lastMeasure[sc]);
      Serial.print(" ");
    }
    Serial.println(" ");
  }
}

// check if the sensor is not returning a 0xFF within one second deactivate sensor
int checkSensors(SoftwareSerial &sensor, byte vccPin) {
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
    return -1;
  }
  return 0;
}

unsigned int measure(SoftwareSerial &sensor, byte vccPin) {
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

// depending on the minimal measurement make a sound
void soundSignal(unsigned int aCm) {

  if (aCm < 50) {
    digitalWrite(BUZZER_VCC_Pin, HIGH);
    if (aCm > 40) {
      tone(BUZZER_IO_Pin, freq[0]);
      delay(500);
    } else if (aCm > 30) {
      tone(BUZZER_IO_Pin, freq[1]);
      delay(1000);
    } else  if (aCm > 10) { // 0 -  30 cm
      tone(BUZZER_IO_Pin, freq[2]);
      delay(3000);
    }
    noTone(BUZZER_IO_Pin);
    digitalWrite(BUZZER_VCC_Pin, LOW);
  } else {
    noTone(BUZZER_IO_Pin);
  }
}


