///////////////////////////////////////////////////////////////////////////////
// Quadrature input logic.
// 
///////////////////////////////////////////////////////////////////////////////

module quadin(
	input			clk,		// system clock
	input 			reset,		// set count to 0
	input			a,			// quadrature phase a
	input			b,			// quadrature phase b
	output [31:0]	count		// signed position value
    );

	reg [31:0]	count_reg, count_next;
	reg [1:0]	state_reg, state_next;
	wire [1:0] qin;
	
	assign count = count_reg;

	signal_sync  #(.WIDTH(2)) sync (.clkB(clk), .inA({a,b}), .outB(qin));

	always @(posedge clk)
	begin
		if (reset)
		begin
			count_reg <= 0;
			state_reg <= qin;
		end
		else
		begin
			count_reg <= count_next;
			state_reg <= state_next;
		end
	end

	always @(*)
	begin
		count_next = count_reg;
		state_next = qin;

		case ({state_reg,qin})
		4'b0001: count_next = count_reg+1;
		4'b0010: count_next = count_reg-1;
		4'b0100: count_next = count_reg-1;
		4'b0111: count_next = count_reg+1;
		4'b1011: count_next = count_reg-1;
		4'b1000: count_next = count_reg+1;
		4'b1101: count_next = count_reg-1;
		4'b1110: count_next = count_reg+1;
		endcase
	end

endmodule

