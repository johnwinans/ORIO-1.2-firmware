module mojo_top(
    input clk,			// 50 MHZ
    input rst_n,		// active low reset
    //output[7:0]led,	// onboard LEDs
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
	 //input [4:0]	quad_a,
	 input quad_a0,
	 input quad_a1,
	 input quad_a2,
	 input quad_a3,
	 input quad_a4,

	 //input [4:0]	quad_b
	 input quad_b0,
	 input quad_b1,
	 input quad_b2,
	 input quad_b3,
	 input quad_b4
    );

	//wire rst = ~rst_n; // make reset active high
	wire rst = 1'b0;

	wire spi_miso_lz;		// internal low-Z output signal
	// MISO should be high-z when not selected
	assign spi_miso = spi_miso_lz; //(spi_ss==0) ? spi_miso_lz : 1'bz;

	wire [31:0] quad_ctr0;
	wire [31:0] quad_ctr1;
	wire [31:0] quad_ctr2;
	wire [31:0] quad_ctr3;
	wire [31:0] quad_ctr4;

	assign {led7,led6,led5,led4,led3,led2,led1,led0} = quad_ctr0[7:0];	// show the lsb of quad #0
	reg [31:0] time_reg;
	reg [5:0] mod50_reg;

	always @(posedge clk)
	begin
		if (rst)
		begin
			mod50_reg <= 0;
			time_reg <= 0;
		end else begin
			if (mod50_reg == 50)
			begin
				mod50_reg <=0;
				time_reg <= time_reg + 1;
			end
			else
				mod50_reg <= mod50_reg + 1;
		end
	end

	SPI_slave #( .BIT_WIDTH(7*4*8) )	// 224 bits!
	spi(
    .reset(rst), 
    .clk(clk), 
    .sck(spi_sck), 
    .ssel(spi_ss), 
    .mosi(spi_mosi), 
    .miso(spi_miso_lz), 
    .rx_data_tick(rx_data_tick), 
    .rx_data(rx_data),
    .tx_data({time_reg, quad_ctr0, quad_ctr1, quad_ctr2, quad_ctr3, quad_ctr4, 32'b0})		// XXX last word = CRC
    );

	quadin quad0 (
    .clk(clk), 
    .reset(rst), 
    .a(quad_a0), 
    .b(quad_b0), 
    .count(quad_ctr0)
    );
	quadin quad1 (
    .clk(clk), 
    .reset(rst), 
    .a(quad_a1), 
    .b(quad_b1), 
    .count(quad_ctr1)
    );
	quadin quad2 (
    .clk(clk), 
    .reset(rst), 
    .a(quad_a2), 
    .b(quad_b2), 
    .count(quad_ctr2)
    );
	quadin quad3 (
    .clk(clk), 
    .reset(rst), 
    .a(quad_a3), 
    .b(quad_b3), 
    .count(quad_ctr3)
    );
	quadin quad4 (
    .clk(clk), 
    .reset(rst), 
    .a(quad_a4), 
    .b(quad_b4), 
    .count(quad_ctr4)
    );

endmodule
