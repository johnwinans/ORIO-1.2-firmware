module top(
    input clk,			// 50 MHZ
    input rst_n,		// active low reset

    // SPI connections
    output spi_miso,	// we send MSB first
    input spi_mosi,
    input spi_ss,		// active-low select
    input spi_sck,		// sample falling, change rising

    //output[7:0]led,	// onboard LEDs
    output led0,
    output led1,
    output led2,
    output led3,
    output led4,
    output led5,
    output led6,
    output led7,
    output led8,
    output led9,

	output pwm0,
	output pwm1,
	output pwm2,
	output pwm3,
	output pwm4,
	output pwm5,
	output pwm6,
	output pwm7,
	output pwm8,
	output pwm9
    );

	localparam SPI_WIDTH = 16*10;

	//wire rst = ~rst_n; // make reset active high
	wire rst = 1'b0;

	wire rx_data_tick;
	wire [SPI_WIDTH-1:0] rx_data;
	reg [SPI_WIDTH-1:0] rx_data_reg;

	wire [9:0] pwm;

	//SPI_slave #( .BIT_WIDTH(7*4*8) )	// 224 bits!
	SPI_slave #( .BIT_WIDTH(SPI_WIDTH) )
	spi(
    .reset(rst), 
    .clk(clk), 
    .sck(spi_sck), 
    .ssel(spi_ss), 
    .mosi(spi_mosi), 
    .miso(spi_miso), 
    .rx_data_tick(rx_data_tick), 
    .rx_data(rx_data),
    .tx_data(16'b0)
    );

/*
	pwm pwm1_unit (
		.clk(clk),
		.rst(rst),
		.duty(rx_data_reg),
		.out(pwm[0])
	);
*/

	assign {led9,led8,led7,led6,led5,led4,led3,led2,led1,led0} = ~rx_data_reg[9:0];	
	always @(posedge rx_data_tick)
	begin
		rx_data_reg = rx_data;
	end

	assign pwm0 = pwm[0];
	assign pwm1 = pwm[1];
	assign pwm2 = pwm[2];
	assign pwm3 = pwm[3];
	assign pwm4 = pwm[4];
	assign pwm5 = pwm[5];
	assign pwm6 = pwm[6];
	assign pwm7 = pwm[7];
	assign pwm8 = pwm[8];
	assign pwm9 = pwm[9];

	genvar i;
	generate
		for (i = 0; i < 10; i = i + 1) begin: genpwm
		    pwm unit  (
        		.clk(clk),
        		.rst(rst),
        		.duty(rx_data_reg[(SPI_WIDTH-1-(i*16)):(SPI_WIDTH-16-(i*16))]),
        		.out(pwm[i])
    		);
			//assign pwm[i] = 0;
		end
	endgenerate

endmodule
