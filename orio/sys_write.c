//**************************************************************************
//
//	Open Robot I/O
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

#include "peripherals.h"

/**
 * This will sink the output from Redlib's printf() and route it to USART_1
 *
 * See "MCUXpresso IDE User Guide", Rev. 11.0.0 - 30 August, 2019, page 157.
 *
 * @param iFileHandle This is not used.
 * @param pcBuffer The buffer of bytes to be written.
 * @param iLength The the number of bytes to be written.
 *
 * @return 0 for success or the number of bytes unwritten if an error 
 *	has occurred.
 ***************************************************************************/
int __sys_write(int iFileHandle, char *pcBuffer, int iLength)
{
	for (int i=0; i<iLength; ++i)
	{
		if (pcBuffer[i] == '\n')
			USART_WriteBlocking(USART_1_PERIPHERAL, (uint8_t*)"\r", 1);
		USART_WriteBlocking(USART_1_PERIPHERAL, (uint8_t*) &pcBuffer[i], 1);
	}
	return 0;
}
