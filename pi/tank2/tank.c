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

#include <zlib.h>


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
 * Reads a joystick event from the joystick device.
 *
 * Returns 0 on success. Otherwise -1 is returned.
*************************************************************************/
int read_event(int fd, struct js_event *event)
{
    ssize_t bytes;

    bytes = read(fd, event, sizeof(*event));

    if (bytes == sizeof(*event))
        return 0;

    /* Error, could not read full event. */
    return -1;
}

/**
 * Returns the number of axes on the controller or 0 if an error occurs.
*************************************************************************/
size_t get_axis_count(int fd)
{
    __u8 axes;

    if (ioctl(fd, JSIOCGAXES, &axes) == -1)
        return 0;

    return axes;
}

/**
 * Returns the number of buttons on the controller or 0 if an error occurs.
*************************************************************************/
size_t get_button_count(int fd)
{
    __u8 buttons;
    if (ioctl(fd, JSIOCGBUTTONS, &buttons) == -1)
        return 0;

    return buttons;
}

/**
***************************************************************************/
size_t get_axis_state(struct js_event *event, int16_t axes[MAX_AXES])
{
	int axis = event->number;
    if (axis < MAX_AXES)
		axes[axis] = event->value;

    return axis;
}


struct odlc_frame
{
    uint8_t     version;        // set to 0
    uint8_t     ftype;          // the type of frame: 1=data, 2=command
    union
    {
        struct
        {
            uint8_t     data[1024];
            uint8_t     crc[4];
        } d;
        struct
        {
            uint8_t     cmd;    // 0=reset, 1=ACK, 2=NAK
            uint8_t     pad;    // make crc start on a 16-bit boundary
            uint8_t     crc[4];
        } c;
    } u;
};

#define ODLC_MAX_FRAME_SIZE     (sizeof(struct odlc_frame))
#define ODLC_FTYPE_DATA         (1)
#define ODLC_FTYPE_COMMAND      (2)
#define ODLC_COMMAND_RESET      (0)
#define ODLC_COMMAND_ACK        (1)
#define ODLC_COMMAND_NAK        (2)


/**
***************************************************************************/
static void transfer(int fd, int16_t left, int16_t right)
{
	int ret;

	struct odlc_frame tx;
	struct odlc_frame rx;

	static int led = 0x100;


	memset(&tx, 0, sizeof(tx));
	memset(&rx, 0, sizeof(rx));

	tx.version = 0;
    tx.ftype = 2;
    tx.u.c.cmd = ODLC_COMMAND_NAK;
    tx.u.c.pad = 0;

    uint32_t i = crc32(0, (unsigned char *)&tx, 4);
    tx.u.c.crc[0] = (i>>24) & 0x0ff;            // big endian CRC32
    tx.u.c.crc[1] = (i>>16) & 0x0ff;
    tx.u.c.crc[2] = (i>>8) & 0x0ff;
    tx.u.c.crc[3] = (i>>0) & 0x0ff;

	uint16_t xferLength = 8;

	led >>= 1;
	if (!led)
		led = 0x100;

	struct spi_ioc_transfer tr = {
		.tx_buf = (unsigned long)&tx,
		.rx_buf = (unsigned long)&rx,
		.len = xferLength,
		.delay_usecs = delay,
		.speed_hz = speed,
		.bits_per_word = bits,
	};

	ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
	if (ret < 1)
		pabort("can't send spi message");

#define DEBUG_PRINT
#ifdef DEBUG_PRINT
	printf("TX:\n");
	hexDump(&tx, xferLength, 0); //sizeof(tx), 0);

	printf("RX:\n");
	hexDump(&rx, xferLength, 0); //sizeof(rx), 0);

#endif

// XXX
	sleep(2);
	//usleep(500000);	// XXX wait for the ORIO to print its message status
}

/**
* Set the tank tred motor. 
* @param right True if setting the right tred speed else left.
* @param speed The motor speed from -32767 to +32767.
****************************************************************************/
static void tankSetSpeed(int16_t speed[2])
{
	transfer(spifd, speed[0], speed[1]);
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
    const char *device;
    int js;
    struct js_event event;
    int16_t axes[MAX_AXES] = {0};
	int16_t tank[2] = {0};
    size_t axis;

	spiInit();

#if 1
	while(1)
		transfer(spifd, 0, 0);

#endif


    if (argc > 1)
        device = argv[1];
    else
        device = "/dev/input/js0";

    js = open(device, O_RDONLY);

	if (js == -1)
		pabort("Could not open joystick");

	//printf("axis count=%d\n", get_axis_count(js));
	//printf("button count=%d\n", get_button_count(js));

    // This will exit if the joystick is 
    while (read_event(js, &event) == 0)
    {
        switch (event.type)
        {
		case JS_EVENT_BUTTON:
            printf("Button %u %s\n", event.number, event.value ? "pressed" : "released");
            break;

        case JS_EVENT_AXIS:
            axis = get_axis_state(&event, axes);

#ifdef DEBUG_PRINT
            if (axis < MAX_AXES)
                printf("Axis %zu at %d\n", axis, axes[axis]);
#endif

			if (axis == 1)
			{
				tank[0] = axes[axis];
				tankSetSpeed(tank);
			}
			else if (axis == 4)
			{
				tank[1] = axes[axis];
				tankSetSpeed(tank);
			}
 			break;

		default:
            break;
        }
        fflush(stdout);
    }

    close(js);
	close(spifd);

    return 0;
}
