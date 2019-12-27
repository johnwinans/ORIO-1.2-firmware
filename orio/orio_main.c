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

#include "orio_adc.h"
#include "orio_spi.h"
#include "orio_pwm.h"

/**
 *
 ******************************************************************/
int main(void)
{
	// XXX For some reason the MCUXpresso code doews not power up the ADC???
	orio_adc_EnablePower();

    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitBootPeripherals();

	orio_pwm_Init();

    printf("ORIO Startup Successful!\nBuild: %s, %s\n", __DATE__, __TIME__);

    spitest();	// This never returns

    return 0 ;
}
