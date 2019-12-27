//**************************************************************************
//
//  Open Robot I/O
//
//    Copyright (C) 2019 John Winans
//
//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program.  If not, see <https://www.gnu.org/licenses/>.
//
//**************************************************************************

#include <stdio.h>
#include "board.h"
#include "peripherals.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "LPC54606.h"

#include "fsl_power.h"
#include "fsl_gpio.h"
#include "fsl_spi.h"

#include "orio_dio.h"
#include "orio_pwm.h"
#include "hexdump.h"


#if 0
/**
 * @param data Buffer of data to calculate.
 * @param dataSize number of bytes in 'data'
 ******************************************************************/
static uint32_t crc16(const void *data, size_t dataSize)
{
	CRC_1_PERIPHERAL->SEED = 0xffff;
	CRC_WriteData(CRC_1_PERIPHERAL, (const uint8_t*)data, dataSize);
	return CRC_1_PERIPHERAL->SUM;
}
#else

/**
 * Calculate a CRC16 of an array of 16-bit words.
 * The data array is byte-swapped while read.
 *
 * @param data Buffer of data to calculate.
 * @param dataSize number of 16-bit words in 'data'
 ******************************************************************/
static uint32_t crc16_16le(const void *data, size_t dataSize)
{
	CRC_1_PERIPHERAL->SEED = 0xffff;
	uint16_t *p = (uint16_t*)data;
	for (int i=0; i<dataSize; ++i)
		*((__O uint16_t *)&(CRC_1_PERIPHERAL->WR_DATA)) = (p[i]>>8)|(p[i]<<8);
	return CRC_1_PERIPHERAL->SUM;
}
#endif
/**
 * @param len Message length expressed in 16-bit words.
 *
 * @return true If the message is valid and has been processed.
 * @return false If the message is invalid in some way.
 ******************************************************************/
static bool processControlMessage(const uint16_t *buf, uint16_t len)
{
#if 0
	hexDump(buf, len*2);
	printf("len=%d, 0: %5d, 1: %5d, DIO=%04x\n", len, (int16_t)buf[4], (int16_t)buf[5], buf[14]);
#endif

	// check the message integrity.
	if (len < 19)
	{
		printf("Invalid message received. Length=%d, expect 38\n", len*2);
		return false;
	}
	if (buf[0] != 0xa55a)
	{
		printf("Invalid message received. Header invalid. Want 0xaa55, got 0x%04x\n", buf[0]);
		return false;
	}
	if (buf[1] != 0x0001)
	{
		printf("Invalid message received. Message type invalid. Want 0x0001, got 0x%04x\n", buf[1]);
		return false;

	}

	// verify the CRC in the received message
	uint32_t crc = crc16_16le(buf, len-1);
	uint32_t mcrc = buf[18];
	if (crc != mcrc)
	{
		printf("CRC FAIL.  Got %04x, want %04x\n", crc, mcrc);
		hexDump(buf, len*2);
		return false;
	}

	// set the PWMs
	for (int i=0; i<10; ++i)
	{
		int32_t v = ((int16_t)buf[4+i]);	// -32768..32767
		v = v/64;							// -512..511
		v += 1024+512;						// 1024..2047
		orio_pwm_Set(i, v);
	}

	// set the digital outputs
	setDIO(buf[14]);
	setLED(buf[15]);
	setRLY(buf[16]);
	setSOL(buf[17]);

	return true;
}

/**
 * Write the given uint16 to the SPI and also to the CRC generator.
 ******************************************************************/
static void spiTX16(uint16_t i)
{
	SPI_WriteData(SPI_1_PERIPHERAL, i, 0);
#if 0
	*((volatile uint16_t *)&(CRC_1_PERIPHERAL->WR_DATA)) = i;
#else
	*((volatile uint8_t *)&(CRC_1_PERIPHERAL->WR_DATA)) = (uint8_t)(i>>8&0x0ff);
	*((volatile uint8_t *)&(CRC_1_PERIPHERAL->WR_DATA)) = (uint8_t)(i&0x0ff);
#endif
}

/**
 *
 ******************************************************************/
static void spiTxHeader(uint16_t type, uint16_t len)
{
	CRC_1_PERIPHERAL->SEED = 0xffff;
    spiTX16(0xa55a);		// message header
    spiTX16(type);			// message type
    spiTX16(len);			// length
    spiTX16(0);				// padding
}

/**
 *
 ******************************************************************/
void spitest()
{
    // Make the ADC free-run collecting samples that we can get any time
    ADC_EnableConvSeqABurstMode(ADC_1_PERIPHERAL, true);

	SPI_Type *base = SPI_1_PERIPHERAL;
	printf("SPI message loop entered\n");

	// prime the FIFO for the next transfer
    base->FIFOCFG |= SPI_FIFOCFG_EMPTYTX_MASK | SPI_FIFOCFG_EMPTYRX_MASK;
    base->FIFOSTAT |= SPI_FIFOSTAT_TXERR_MASK | SPI_FIFOSTAT_RXERR_MASK;

    spiTxHeader(0x0002, 36);

	uint16_t buf[100] = {0};
	int pos = 0;
	while(1)
	{
		if (base->FIFOSTAT & SPI_FIFOSTAT_RXNOTEMPTY_MASK)
		{
			if(pos < sizeof(buf)/sizeof(buf[0]))
				buf[pos] = SPI_ReadData(base)&0xffff;

			// XXX read and toss the data if there is nowhere to put it. XXX

			switch(pos)
			{
			case 0:
				spiTX16(getSwitch());
				break;

			case 1:
			case 2:
			case 3:
			case 4:
			case 5:
			case 6:
			case 7:
			case 8:
			case 9:
				spiTX16(ADC0->DAT[pos+2]&0x0ffff);	// send the data in raw ADC units
				// This takes too long to calculate in real time... the the user deal with it :-D
//				SPI_WriteData(base, (uint16_t)(((ADC0->DAT[pos+2]&0x0ffff)*(3.3/0x10000)*2.0)*1000), 0);
				break;

			case 10:
				spiTX16(getDIO());
					break;

			case 11:
				{
					// this is tolerable because we only prime the FIFO with 4 words (and it can hold 8)
					uint32_t crc = CRC_1_PERIPHERAL->SUM;
					spiTX16(crc&0x0ffff);
				}
				break;

			default:
				spiTX16(0x1111);
			}

			++pos;
		}
		if (base->STAT & SPI_STAT_SSA_MASK)
		{
			// SSEL just became active (start of message)
			base->STAT = SPI_STAT_SSA_MASK;
			pos = 0;
		}
		if (base->STAT & SPI_STAT_SSD_MASK)
		{
			// SSEL just became inactive (end of message)
			base->STAT = SPI_STAT_SSD_MASK;     // clear the slave select de-select status

#if 0
			uint32_t fifostat = base->FIFOSTAT;
#endif
			// prime the FIFO for the next transfer
		    base->FIFOCFG |= SPI_FIFOCFG_EMPTYTX_MASK | SPI_FIFOCFG_EMPTYRX_MASK;
		    base->FIFOSTAT |= SPI_FIFOSTAT_TXERR_MASK | SPI_FIFOSTAT_RXERR_MASK;

			int elements = pos>=sizeof(buf)/sizeof(buf[0])?sizeof(buf)/sizeof(buf[0]):pos;
#if 0
			hexDump(buf, elements*2);
			printf("stat=%08x, len=%d, 0: %5d, 1: %5d, DIO=%04x\n", fifostat, pos, (int16_t)buf[2], (int16_t)buf[3], buf[12]);
			printf("CRC=%08x\n", CRC_1_PERIPHERAL->SUM);
#endif

			processControlMessage(buf, elements);

			spiTxHeader(0x0002, 36);
		}
	}
}
