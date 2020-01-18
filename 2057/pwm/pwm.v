module pwm
	#(
		parameter PERIOD=(65536)		// total clock ticks per period
	)
	(
    input clk,							// 
    input rst,							// active high reset
	input [$clog2(PERIOD)-1:0] duty,	// number of ticks of high time
	output out							// pwm modulated output
    );

	reg out_reg;
	reg out_next;
	assign out = out_reg;

	reg [$clog2(PERIOD)-1:0] ctr_reg;
	reg [$clog2(PERIOD)-1:0] ctr_next;

	always @(posedge clk)
	begin
		if (rst)
		begin
			ctr_reg <= 0;
			out_reg <= 0;
		end
		else
		begin
			ctr_reg <= ctr_next;
			out_reg <= out_next;
		end
	end

	always @(*)
	begin
		ctr_next = (ctr_reg < PERIOD) ? (ctr_reg + 1) : 0;
		out_next = (duty > ctr_reg) ? 1 : 0;
	end

endmodule
