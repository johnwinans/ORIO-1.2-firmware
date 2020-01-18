module top(
    input clk,			// 25 MHZ
    input rst_n,		// active low reset
    output led0,	
    output led1,	
    output led2,	
    output led3,	
    output led4,	
    output led5,	
    output led6,	
    output led7
    );

	wire rst = ~rst_n; // make reset active high

	reg [19:0] time_reg;

	//assign {led7,led6,led5,led4,led3,led2,led1,led0} = time_reg[19:12];	
	assign led0 = time_reg[0];
	assign led1 = time_reg[1];
	assign led2 = time_reg[2];
	assign led3 = time_reg[3];
	assign led4 = time_reg[4];
	assign led5 = time_reg[5];
	assign led6 = time_reg[6];
	assign led7 = time_reg[7];

	always @(posedge clk)
	begin
		if (rst)
			time_reg <= 0;
		else
			time_reg <= time_reg + 1;
	end

endmodule
