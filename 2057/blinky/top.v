module top(
    input wire clk,			// 25 MHZ
    input wire rst_n,		// active low reset
	output wire[7:0]  led
    );

	// Turn on a pull up resistor on the reset input pin
	wire rst_n_p;
	SB_IO #(
		.PIN_TYPE(6'b0000_01),	// output = 0, input = 1
		.PULLUP(1'b1)			// enable the pullup = 1
	) reset_button(
		.PACKAGE_PIN(rst_n),	// the physical pin number with the pullup on it
		.D_IN_0(rst_n_p)			// an internal wire for this pin
	);

	wire rst = ~rst_n_p; // make reset active high
	reg [27:0] time_reg;
	assign led[7:0] = time_reg[27:20];

	always @(posedge clk)
	begin
		if (rst)
			time_reg <= 0;
		else
			time_reg <= time_reg + 1;
	end

endmodule
