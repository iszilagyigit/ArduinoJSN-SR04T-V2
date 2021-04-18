
/*
  SPI example wit wiring pi
  
  From: https://roboticsbackend.com/raspberry-pi-master-arduino-uno-slave-spi-communication-with-wiringpi/
  
  g++  spi-wpi.c -o spi-wpi -lwiringPi -lcrypt -pthread -lrt
*/

#include <iostream>
#include <wiringPiSPI.h>

#define SPI_CHANNEL 0
#define SPI_CLOCK_SPEED 1000000

int main(int argc, char **argv)
{
    int fd = wiringPiSPISetupMode(SPI_CHANNEL, SPI_CLOCK_SPEED, 0);
    if (fd == -1) {
        std::cout << "Failed to init SPI communication.\n";
        return -1;
    }
    std::cout << "SPI communication successfully setup.\n";
   
    unsigned char buf[4] = { 23, 0, 0, 0 };
    wiringPiSPIDataRW(SPI_CHANNEL, buf, 4);

    std::cout << "Data returned: " << + buf[1] << "\n";
    std::cout << "Data returned: " << + buf[2] << "\n";
    std::cout << "Data returned: " << + buf[3] << "\n";
    return 0;
}