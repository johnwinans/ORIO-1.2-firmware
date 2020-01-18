// Copied from fpga4fun.com
// Altered to support a paramaterized transfer buffer size

module SPI_slave
	#(
		parameter BIT_WIDTH=(8)			// SPI message size in bits
	)
	(
	input wire	reset,
	input wire	clk,
	input wire	sck, 
	input wire	ssel, 
	input wire	mosi,
	output wire	miso,

	output wire rx_data_tick,
	output wire [BIT_WIDTH-1:0] rx_data,

	input wire [BIT_WIDTH-1:0] tx_data		// must be stable on posedge clk
	);

	// sync SCK to the FPGA clock using a 3-bit shift register
	reg [2:0] SCKr;  always @(posedge clk) SCKr <= {SCKr[1:0], sck};
	wire SCK_risingedge = (SCKr[2:1]==2'b01);  // now we can detect SCK rising edges
	wire SCK_fallingedge = (SCKr[2:1]==2'b10);  // and falling edges

	// same thing for SSEL
	reg [2:0] SSELr;  always @(posedge clk) SSELr <= {SSELr[1:0], ssel};
	wire SSEL_active = ~SSELr[1];  // SSEL is active low
	wire SSEL_startmessage = (SSELr[2:1]==2'b10);  // message starts at falling edge
	wire SSEL_endmessage = (SSELr[2:1]==2'b01);  // message stops at rising edge

	// and for MOSI
	reg [1:0] MOSIr;  always @(posedge clk) MOSIr <= {MOSIr[0], mosi};
	wire MOSI_data = MOSIr[1];

	reg [$clog2(BIT_WIDTH)-1:0] bitcnt;		// counts the number of bits in the message being transferred

	reg msg_received;  // high when a message been received
	reg [BIT_WIDTH-1:0] msg_data_received;
	
	always @(posedge clk)
	begin
		if (reset || ~SSEL_active)
		begin
			bitcnt <= 3'b0;
			msg_data_received <= 3'b0;
  		end
  		else
  			if(SCK_risingedge)
  			begin
    			bitcnt <= bitcnt + 3'b001;

				// data arrives MSB first
    			msg_data_received <= {msg_data_received[BIT_WIDTH-2:0], MOSI_data};
  			end
	end

	always @(posedge clk) 
		msg_received <= (SSEL_active && SCK_risingedge && (bitcnt==(BIT_WIDTH-1)));
			
	// XXX we assume that there is only one slave on the SPI bus
	// so we don't bother with a tri-state buffer for MISO
	// otherwise we would need to tri-state MISO when SSEL is inactive

	assign rx_data_tick = msg_received;
	assign rx_data = msg_data_received;


	reg [BIT_WIDTH-1:0] msg_data_sent;

	always @(posedge clk)
		if(SSEL_active)
		begin
			if(SSEL_startmessage)
				msg_data_sent <= tx_data;
			else
				if(SCK_fallingedge)
				begin
//					if(bitcnt==3'b000)
//						msg_data_sent <= 8'h00;  // after that, we send 0s
//					else
						msg_data_sent <= {msg_data_sent[BIT_WIDTH-2:0], 1'b0};
				end
		end
	
	assign miso = msg_data_sent[BIT_WIDTH-1];  // send MSB first

endmodule

