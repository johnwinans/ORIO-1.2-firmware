module top(
    input clk25,			// 25 MHZ
    input rst_n,		// active low reset

	output refclk,

    output led0,
    output led1,
    output led2,
    output led3,
    output led4,
    output led5,
    output led6,
    output led7,

    // RoboRIO SPI connections
    output spi_miso,	// we send MSB first
    input spi_ss,		// active-low select
    input spi_mosi,
    input spi_sck,	// sample falling, change rising
	 
	 // quadrature inputs
	 input quad_a0,
	 input quad_a1,
	 input quad_a2,
	 input quad_a3,
	 input quad_a4,
	 input quad_a5,
	 input quad_a6,
	 input quad_a7,
	 input quad_a8,
	 input quad_a9,

	 input quad_b0,
	 input quad_b1,
	 input quad_b2,
	 input quad_b3,
	 input quad_b4,
	 input quad_b5,
	 input quad_b6,
	 input quad_b7,
	 input quad_b8,
	 input quad_b9

    );

	wire rstp;			// reset with a pullup
	wire rst = ~rstp;	// make rst an active high signal

	assign {led7,led6,led5,led4,led3,led2,led1,led0} = quad_ctr0[9:0];	// show the lsb of quad #0


	// Turn on a pull up resistor on the reset input pin
	SB_IO #(
		.PIN_TYPE(6'b0000_01),	// output = 0, input = 1
		.PULLUP(1'b1)			// enable the pullup = 1
	) reset_button(
		.PACKAGE_PIN(rst_n),	// the physical pin number with the pullup on it
		.D_IN_0(rstp)			// an internal wire for this pin
	);


	// generate a 100MHZ system clock
	wire clk;
	pll_25_100 upll(.clock_in(clk25), .global_clock(clk));


	wire spi_miso_lz;		// internal low-Z output signal
	// MISO should be high-z when not selected
	assign spi_miso = spi_miso_lz; //(spi_ss==0) ? spi_miso_lz : 1'bz;


	// A 1mhz timer for the uptime in the SPI resonse messages
	reg [31:0] time_reg;
	reg [4:0] mod25_reg;

	always @(posedge clk)
	begin
		if (rst)
		begin
			mod25_reg <= 0;
			time_reg <= 0;
		end else begin
			if (mod25_reg == 25)
			begin
				mod25_reg <=0;
				time_reg <= time_reg + 1;
			end
			else
				mod25_reg <= mod25_reg + 1;
		end
	end


	//SPI_slave #( .BIT_WIDTH((2*4+10*2)*8) )
	SPI_slave #( .BIT_WIDTH((2*4+10*4)*8) )
	spi(
    .reset(rst), 
    .clk(clk), 
    .sck(spi_sck), 
    .ssel(spi_ss), 
    .mosi(spi_mosi), 
    .miso(spi_miso_lz), 
    .rx_data_tick(rx_data_tick), 
    .rx_data(rx_data),
    .tx_data({time_reg, quad_ctr0, quad_ctr1, quad_ctr2, quad_ctr3, quad_ctr4, quad_ctr5, quad_ctr6, quad_ctr7, quad_ctr8, quad_ctr9, 32'b0})		// XXX last word = CRC
    );

	
	// generate the reference clock for the quadrature sampler
	//wire qclk = time_reg[1];	// 500khz
	//wire qclk = time_reg[2];	// 250khz
	//wire qclk = time_reg[3];	// 125khz
	wire qclk = time_reg[4];	//  62.5khz
	assign refclk = qclk;

	wire [31:0] quad_ctr0;
	wire [31:0] quad_ctr1;
	wire [31:0] quad_ctr2;
	wire [31:0] quad_ctr3;
	wire [31:0] quad_ctr4;
	wire [31:0] quad_ctr5;
	wire [31:0] quad_ctr6;
	wire [31:0] quad_ctr7;
	wire [31:0] quad_ctr8;
	wire [31:0] quad_ctr9;

	quadinx quad0 (
    .clk(qclk),
    .reset(rst),
    .a(quad_a0),
    .b(quad_b0),
    .count(quad_ctr0)
    );
	quadinx quad1 (
    .clk(qclk),
    .reset(rst),
    .a(quad_a1),
    .b(quad_b1),
    .count(quad_ctr1)
    );
	quadinx quad2 (
    .clk(qclk),
    .reset(rst),
    .a(quad_a2),
    .b(quad_b2),
    .count(quad_ctr2)
    );
	quadinx quad3 (
    .clk(qclk),
    .reset(rst),
    .a(quad_a3),
    .b(quad_b3),
    .count(quad_ctr3)
    );
	quadinx quad4 (
    .clk(qclk),
    .reset(rst),
    .a(quad_a4),
    .b(quad_b4),
    .count(quad_ctr4)
    );

	quadinx quad5 (
    .clk(qclk),
    .reset(rst),
    .a(quad_a5),
    .b(quad_b5),
    .count(quad_ctr5)
    );
	quadinx quad6 (
    .clk(qclk),
    .reset(rst),
    .a(quad_a6),
    .b(quad_b6),
    .count(quad_ctr6)
    );
	quadinx quad7 (
    .clk(qclk),
    .reset(rst),
    .a(quad_a7),
    .b(quad_b7),
    .count(quad_ctr7)
    );
	quadinx quad8 (
    .clk(qclk),
    .reset(rst),
    .a(quad_a8),
    .b(quad_b8),
    .count(quad_ctr8)
    );
	quadinx quad9 (
    .clk(qclk),
    .reset(rst),
    .a(quad_a9),
    .b(quad_b9),
    .count(quad_ctr9)
    );

endmodule
