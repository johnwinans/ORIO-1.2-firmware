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

#include "hexdump.h"

#include <stdio.h>
#include <stdint.h>


void hexDump(const void *a, unsigned long len)
{
	const unsigned char *p = (const unsigned char*)a;
    if (((uint32_t)p)%16 != 0)
    {
        printf("%8x:    ", (uint32_t)p);
        int i = ((uint32_t)p)%16;
        while(--i)
            printf("   ");
    }
	while(len)
	{
		if (((uint32_t)p)%16 == 0)
			printf("\n%8x: ", (uint32_t)p);
		printf("%s%02x", (((uint32_t)p)%16 == 8)?"-":" ", *p);
		++p;
		--len;
	}
    printf("\n");
}
