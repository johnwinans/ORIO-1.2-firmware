// Original version from:  https://www.fpga4fun.com/QuadratureDecoder.html

module quadinx(
	input			clk,		// system clock
	input 			reset,		// set count to 0
	input			a,			// quadrature phase a
	input			b,			// quadrature phase b
	output [31:0]	count		// signed position value
	);

//clk, quadA, quadB, count);
//input clk, quadA, quadB;
//output [7:0] count;

reg [2:0] quadA_delayed, quadB_delayed;
always @(posedge clk) quadA_delayed <= {quadA_delayed[1:0], a};
always @(posedge clk) quadB_delayed <= {quadB_delayed[1:0], b};

wire count_enable = quadA_delayed[1] ^ quadA_delayed[2] ^ quadB_delayed[1] ^ quadB_delayed[2];
wire count_direction = quadA_delayed[1] ^ quadB_delayed[2];

reg [31:0] count;
always @(posedge clk)
begin
  if(count_enable)
  begin
    if(count_direction) count<=count+1; else count<=count-1;
  end
end

endmodule 
