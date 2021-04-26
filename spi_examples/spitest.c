
/*
 SPI test for RPI3  with Android.

 RPI3 - SPI master
 Arduino - with 4 distance sensor - slave

expected output with test arduino spi client : jsnr04t-spi.ino: (tag 2.0)
                                                
pi@raspberrypi:~/gitclones/ArduinoJSN-SR04T-V2 $ ./spitest 
SPI-Mode.......: 0
Wortlaenge.....: 8
Geschwindigkeit: 7629 Hz
 response 0:  0x2d 
 response 0:  0x2d 
 response  0xaa 
 response  0x19 
 response  0x2a 
 response  0xfa 
 response  0x2c 
 response  0x2d 
 response  0x2d 

rpi3:/data/test # 

 @author iszilagyi
*/

#include <stdint.h>
#include <linux/spi/spidev.h>   
#include <fcntl.h>				
#include <sys/ioctl.h>			
#include <stdio.h>  // for printf
#include <unistd.h> // for close

int spi_cs0_fd;				//file descriptor for the SPI device
int spi_cs1_fd;				//file descriptor for the SPI device

uint8_t spi_mode; // 8 bit unsigned
uint8_t spi_bitsPerWord;
uint32_t spi_speed;//32 bit unsigned


//spi_device	0=CS0, 1=CS1
int SpiOpenPort (int spi_device)
{
	int status_value = -1;
    int *spi_cs_fd;

    //----- SET SPI MODE -----
    //SPI_MODE_0 (0,0) 	CPOL = 0, CPHA = 0, Clock idle low, data is clocked in on rising edge, output data (change) on falling edge
    spi_mode = SPI_MODE_0;
    
    //----- SET BITS PER WORD -----
    spi_bitsPerWord = 8;
    
    //----- SET SPI BUS SPEED -----
    spi_speed = 7629;		//1000000 = 1MHz ( 1uS per bit) 


    if (spi_device == 1)
    	spi_cs_fd = &spi_cs1_fd;
    else
    	spi_cs_fd = &spi_cs0_fd;


    if (spi_device == 1)
    	*spi_cs_fd = open("/dev/spidev0.1", O_RDWR);
    else
    	*spi_cs_fd = open("/dev/spidev0.0", O_RDWR);

    if (*spi_cs_fd < 0)
    {
        perror("Error - Could not open SPI device");
        return 1;
    }

    status_value = ioctl(*spi_cs_fd, SPI_IOC_WR_MODE, &spi_mode);
    if(status_value < 0)
    {
        perror("Could not set SPIMode (WR)...ioctl fail");
        return 1;
    }

    status_value = ioctl(*spi_cs_fd, SPI_IOC_RD_MODE, &spi_mode);
    if(status_value < 0)
    {
      perror("Could not set SPIMode (RD)...ioctl fail");
      return 1;
    }

    status_value = ioctl(*spi_cs_fd, SPI_IOC_WR_BITS_PER_WORD, &spi_bitsPerWord);
    if(status_value < 0)
    {
      perror("Could not set SPI bitsPerWord (WR)...ioctl fail");
      return 1;
    }

    status_value = ioctl(*spi_cs_fd, SPI_IOC_RD_BITS_PER_WORD, &spi_bitsPerWord);
    if(status_value < 0)
    {
      perror("Could not set SPI bitsPerWord(RD)...ioctl fail");
      return 1;
    }

    status_value = ioctl(*spi_cs_fd, SPI_IOC_WR_MAX_SPEED_HZ, &spi_speed);
    if(status_value < 0)
    {
      perror("Could not set SPI speed (WR)...ioctl fail");
      return 1;
    }

    status_value = ioctl(*spi_cs_fd, SPI_IOC_RD_MAX_SPEED_HZ, &spi_speed);
    if(status_value < 0)
    {
      perror("Could not set SPI speed (RD)...ioctl fail");
      return 1;
    }
    
  //  printf("SPI-Device.....: %s\n", spi_device);
	printf("SPI-Mode.......: %d\n",spi_mode);
	printf("Wortlaenge.....: %d\n",spi_bitsPerWord);
	printf("Geschwindigkeit: %d Hz\n",  spi_speed);

    return 0;
}

int SpiClosePort (int spi_device)
{
	int status_value = -1;
    int *spi_cs_fd;

    if (spi_device)
    	spi_cs_fd = &spi_cs1_fd;
    else
    	spi_cs_fd = &spi_cs0_fd;


    status_value = close(*spi_cs_fd);
    if(status_value < 0)
    {
    	perror("Error - Could not close SPI device");
    	return 1;
    }
    return 0;
}

// spi_device  0 or 1
// data		Bytes to write.  Contents is overwritten with bytes read.
int SpiWriteAndRead (int spi_device, uint32_t *data)
{
	struct spi_ioc_transfer spi[1];
	int retVal = -1;
    int *spi_cs_fd;

    if (spi_device)
    	spi_cs_fd = &spi_cs1_fd;
    else
    	spi_cs_fd = &spi_cs0_fd;

	//one spi transfer for each byte
		spi[0].tx_buf        =  (uint32_t) data; // transmit from "data"
		spi[0].rx_buf        =  (uint32_t) data; // receive into "data"
		spi[0].len           = 1;
		spi[0].delay_usecs   = 0 ; //TODO check if is needed to increase it
		spi[0].speed_hz      = spi_speed ;
		spi[0].bits_per_word = spi_bitsPerWord ;
		spi[0].cs_change = 0;
        spi[0].pad = 0; 
		
	

	retVal = ioctl(*spi_cs_fd, SPI_IOC_MESSAGE(1), &spi) ;

	if(retVal < 0)
	{
		perror("Error - Problem transmitting spi data..ioctl");
		return -1;
	}
	return retVal;
}

int main(void) {

	int op1 = SpiOpenPort(0);
	if (op1 == 1) {
		return op1;
	}
    //for (uint8_t j=0;j<=5;j++) {

	uint32_t data = 0xFF;
	SpiWriteAndRead(0, &data);
	printf(" response 0:  %#x \r\n", data);
	SpiWriteAndRead(0, &data);
	printf(" response 0:  %#x \r\n", data);
	  
	// 8 bit
	for (uint8_t i  = 0x01; i <= 0x07; i++) {
		data = i <= 0x04 ? i : 0xFF;
		SpiWriteAndRead(0, &data);
		// the 1st response comes only after the 2nd request?! 
		printf(" response  %#x \r\n", data);
	}
	//}
	
	SpiClosePort(0);
    return 0;
}

