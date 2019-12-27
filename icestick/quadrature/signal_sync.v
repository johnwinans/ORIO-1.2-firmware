`timescale 1ns / 1ps

// Synchronize a single bit signal level across two clock domains

module signal_sync #(
		parameter WIDTH = 1
	) (
		input wire [WIDTH-1:0]	inA,	// signal A in clock domain A
    	input wire				clkB,	// clock for domain B
		output wire [WIDTH-1:0]	outB	// inA synchronized to clkB
	);

	reg [WIDTH-1:0] ff1_reg;
	reg [WIDTH-1:0] ff2_reg;

	always @(posedge clkB) 
	begin
		ff1_reg <= inA;
		ff2_reg <= ff1_reg;
	end

	assign outB = ff2_reg;

endmodule

