#include <fcntl.h>				//Needed for SPI port
#include <sys/ioctl.h>			//Needed for SPI port
#include <linux/spi/spidev.h>	//Needed for SPI port
#include <unistd.h>			//Needed for SPI port
#include <stdio.h>
#include <stdlib.h>


int spi_cs0_fd;				//file descriptor for the SPI device
int spi_cs1_fd;				//file descriptor for the SPI device
unsigned char spi_mode; //8bit
unsigned char spi_bitsPerWord;//8bit
unsigned short spi_speed;//32bit



//***********************************
//***********************************
//********** SPI OPEN PORT **********
//***********************************
//***********************************
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
        exit(1);
    }

    status_value = ioctl(*spi_cs_fd, SPI_IOC_WR_MODE, &spi_mode);
    if(status_value < 0)
    {
        perror("Could not set SPIMode (WR)...ioctl fail");
        exit(1);
    }

    status_value = ioctl(*spi_cs_fd, SPI_IOC_RD_MODE, &spi_mode);
    if(status_value < 0)
    {
      perror("Could not set SPIMode (RD)...ioctl fail");
      exit(1);
    }

    status_value = ioctl(*spi_cs_fd, SPI_IOC_WR_BITS_PER_WORD, &spi_bitsPerWord);
    if(status_value < 0)
    {
      perror("Could not set SPI bitsPerWord (WR)...ioctl fail");
      exit(1);
    }

    status_value = ioctl(*spi_cs_fd, SPI_IOC_RD_BITS_PER_WORD, &spi_bitsPerWord);
    if(status_value < 0)
    {
      perror("Could not set SPI bitsPerWord(RD)...ioctl fail");
      exit(1);
    }

    status_value = ioctl(*spi_cs_fd, SPI_IOC_WR_MAX_SPEED_HZ, &spi_speed);
    if(status_value < 0)
    {
      perror("Could not set SPI speed (WR)...ioctl fail");
      exit(1);
    }

    status_value = ioctl(*spi_cs_fd, SPI_IOC_RD_MAX_SPEED_HZ, &spi_speed);
    if(status_value < 0)
    {
      perror("Could not set SPI speed (RD)...ioctl fail");
      exit(1);
    }
    
  //  printf("SPI-Device.....: %s\n", spi_device);
	printf("SPI-Mode.......: %d\n",spi_mode);
	printf("Wortlaenge.....: %d\n",spi_bitsPerWord);
	printf("Geschwindigkeit: %d Hz\n",  spi_speed);

    return(status_value);
}



//************************************
//************************************
//********** SPI CLOSE PORT **********
//************************************
//************************************
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
    	exit(1);
    }
    return status_value;
}



//*******************************************
//*******************************************
//********** SPI WRITE & READ DATA **********
//*******************************************
//*******************************************
//data		Bytes to write.  Contents is overwritten with bytes read.
int SpiWriteAndRead (int spi_device, unsigned char *data)
{
	struct spi_ioc_transfer spi[1];
	int i = 0;
	int retVal = -1;
    int *spi_cs_fd;

    if (spi_device)
    	spi_cs_fd = &spi_cs1_fd;
    else
    	spi_cs_fd = &spi_cs0_fd;

	//one spi transfer for each byte
		spi[0].tx_buf        = (unsigned long)(data + i); // transmit from "data"
		spi[0].rx_buf        = (unsigned long)(data + i) ; // receive into "data"
		spi[0].len           = 1;
		spi[0].delay_usecs   = 0 ;
		spi[0].speed_hz      = spi_speed ;
		spi[0].bits_per_word = spi_bitsPerWord ;
		spi[0].cs_change = 0;
		spi[0].tx_nbits = 8;
		spi[0].rx_nbits = 8; 
		spi[0].pad = 0; 
		
	

	retVal = ioctl(*spi_cs_fd, SPI_IOC_MESSAGE(1), &spi) ;

	if(retVal < 0)
	{
		perror("Error - Problem transmitting spi data..ioctl");
		exit(1);
	}
	printf(" spi.rx %lu ",(unsigned long)(spi[0].rx_buf));
	return retVal;
}

int main(void) {
	printf(" Started ");
	SpiOpenPort(0);
	unsigned char data  = 0x02; //8 bit
	int retVal = SpiWriteAndRead(0, &data);
	printf(" response: %d  %d", data, retVal);
	
	SpiClosePort(0);
	
}
