#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <getopt.h>

#include <arpa/inet.h>	// htons

#include <linux/types.h>
#include <linux/spi/spidev.h>
#include <linux/joystick.h>

#include <zlib.h>

#define MAX_AXES	(32)


static const char *device = "/dev/spidev0.0";
static uint8_t mode;
static uint8_t bits = 8;
static uint32_t speed = 4000000;
//static uint32_t speed = 1000000;
//static uint32_t speed =  500000;
//static uint32_t speed =    250000;
//static uint32_t speed =  100000;
//static uint32_t speed =     50000;

static uint16_t delay = 0;

static int spifd;

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

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
	if (addr%16 != 0)
	{
		printf("%8x:    ", addr);
		int i = addr%16;
		while(--i)
			printf("   ");
	}
	while(len)
	{
		if (addr%16 == 0)
			printf("\r\n%8x: ", addr);
		printf("%s%02x", (addr%16 == 8)?"-":" ", *p);
		++p;
		++addr;
		--len;
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

struct msg1
{
	int16_t		pwm[10];	// 4-13
	uint16_t	dio;		// 14
	uint16_t	led;		// 15
	uint16_t	relay;		// 16
	uint16_t	solenoid;	// 17
};

struct msg2
{
	uint16_t	switches;	// 4
	uint16_t	adc[8];		// 5-12
	uint16_t	vbatt;		// 13
	uint16_t	dio;		// 14
};

struct msg
{
	uint16_t	header;		// 0 0xa55a
	uint16_t	msgType;	// 1
	uint16_t	length;		// 2
	uint16_t	pad;		// 3
	union
	{
		struct msg1	m1;
		struct msg2 m2;
	} u;
	uint32_t	crc;		// CRC32
};

/**
***************************************************************************/
static void transfer(int fd, int16_t left, int16_t right)
{
	int ret;

	struct msg tx = { 0 };
	struct msg rx = { 0 };
	static int led = 0x100;

	tx.header = htons(0xa55a);
	tx.msgType = htons(0x0001);
	tx.length = htons(sizeof(tx));
	tx.u.m1.pwm[0] = htons(right);
	tx.u.m1.pwm[1] = htons(left);
	tx.u.m1.pwm[2]= htons(left);
	tx.u.m1.pwm[3] = htons(left);
	tx.u.m1.pwm[4] = htons(left);
	tx.u.m1.pwm[5] = htons(left);
	tx.u.m1.pwm[6] = htons(left);
	tx.u.m1.pwm[7] = htons(left);
	tx.u.m1.pwm[8] = htons(left);
	tx.u.m1.pwm[9] = htons(left);
	tx.u.m1.dio = htons(0);
	tx.u.m1.led = htons(~led);
	tx.u.m1.relay = htons(led);
	tx.u.m1.solenoid = htons(led);
	tx.crc = htonl(crc32(0xffffffff, (unsigned char*)&tx, sizeof(tx)));

	led >>= 1;
	if (!led)
		led = 0x100;

	struct spi_ioc_transfer tr = {
		.tx_buf = (unsigned long)&tx,
		.rx_buf = (unsigned long)&rx,
		.len = sizeof(tx),
		.delay_usecs = delay,
		.speed_hz = speed,
		.bits_per_word = bits,
	};

	ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
	if (ret < 1)
		pabort("can't send spi message");

	printf("TX:\n");
	hexDump(&tx, sizeof(tx), 0);

	printf("RX:\n");
	hexDump(&rx, sizeof(rx), 0);

	rx.header = ntohs(rx.header);
	rx.msgType = ntohs(rx.msgType);
	rx.length = ntohs(rx.length);

	printf("msg: %04x, %04x, %04x\n", rx.header, rx.msgType, rx.length);

	for (int i=0; i < 8; ++i)
	{
		float f = ntohs(rx.u.m2.adc[i]) * (3.3/0x10000)*2.0;
		printf("%2d: %f\n", i, f);
	}
	float f = ntohs(rx.u.m2.vbatt)*(3.3/0x10000)*(33.0+3.3)/3.3;
	printf("+V: %f\n", f);
	
	printf("DIO: %04x\n", ntohs(rx.u.m2.dio));
	printf(" SW: %04x\n", ntohs(rx.u.m2.switches));

// XXX
//	usleep(25000);	// XXX wait for the ORIO to print its message status
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
            if (axis < MAX_AXES)
                printf("Axis %zu at %d\n", axis, axes[axis]);

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
