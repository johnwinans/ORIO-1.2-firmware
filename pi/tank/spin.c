#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <getopt.h>
#include <string.h>

#include <arpa/inet.h>	// htons

#include <linux/types.h>
#include <linux/spi/spidev.h>
#include <linux/joystick.h>

#include "checksum.h"

#define MAX_AXES	(32)


static const char *device = "/dev/spidev0.0";
static uint8_t mode;
static uint8_t bits = 8;
//static uint32_t speed = 10000000;
//static uint32_t speed = 8000000;
static uint32_t speed = 4000000;
//static uint32_t speed = 1000000;
//static uint32_t speed =  500000;
//static uint32_t speed =  250000;
//static uint32_t speed =  100000;
//static uint32_t speed =   50000;

static uint16_t delay = 0;

static int spifd;


/**
* Print a message and exit/abort the program.
*************************************************************************/
static void pabort(const char *s)
{
	perror(s);
	abort();
}

/**
*************************************************************************/
void hexDump(const void *a, unsigned long len, uint32_t addr)
{
	const unsigned char *p = (const unsigned char*)a;
	int first = 1;
	if (addr%16 != 0)
	{
		first = 0;
		printf("%8x:    ", addr);
		int i = addr%16;
		while(--i)
			printf("   ");
	}
	while(len)
	{
		if (addr%16 == 0)
		{
			if (!first)
				printf("\r\n");
			printf("%8x: ", addr);
		}
		printf("%s%02x", (addr%16 == 8)?"-":" ", *p);
		++p;
		++addr;
		--len;
		first = 0;
	}
	printf("\r\n");
}


/**
***************************************************************************/
static void xact()
{
	int fd = spifd;

	uint8_t tx[1024];
	uint8_t rx[1024];

	struct spi_ioc_transfer tr = {
		.tx_buf = (unsigned long)&tx,
		.rx_buf = (unsigned long)&rx,
		.len = sizeof(tx),
		.delay_usecs = delay,
		.speed_hz = speed,
		.bits_per_word = bits,
	};

	int ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
	if (ret < 1)
		pabort("can't send spi message");
}


/**
***************************************************************************/
int spiInit()
{
	int ret = 0;

	spifd = open(device, O_RDWR);
	if (spifd < 0)
		pabort("can't open device");

	ret = ioctl(spifd, SPI_IOC_WR_MODE, &mode);
	if (ret == -1)
		pabort("can't set spi mode");

	ret = ioctl(spifd, SPI_IOC_RD_MODE, &mode);
	if (ret == -1)
		pabort("can't get spi mode");

	ret = ioctl(spifd, SPI_IOC_WR_BITS_PER_WORD, &bits);
	if (ret == -1)
		pabort("can't set bits per word");

	ret = ioctl(spifd, SPI_IOC_RD_BITS_PER_WORD, &bits);
	if (ret == -1)
		pabort("can't get bits per word");

	ret = ioctl(spifd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
	if (ret == -1)
		pabort("can't set max speed hz");

	ret = ioctl(spifd, SPI_IOC_RD_MAX_SPEED_HZ, &speed);
	if (ret == -1)
		pabort("can't get max speed hz");

	printf("device: %s\n", device);
	printf("spi mode: %d\n", mode);
	printf("bits per word: %d\n", bits);
	printf("max speed: %d Hz (%d KHz)\n", speed, speed/1000);

	return 0;
}


/**
****************************************************************************/
int main(int argc, char *argv[])
{
	spiInit();

	while(1)
	{
		xact();
	}

	close(spifd);
    return 0;
}
