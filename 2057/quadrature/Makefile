# Project setup
PROJ      = quadrature
BUILD     = ./build

# for 1K devices
#DEVICE    = 1k
#FOOTPRINT = tq144
#PINS=pinmap-icestick.pcf

# for 4K devices
DEVICE    = 8k
FOOTPRINT = tq144:4k
PINS=pinmap-2057.pcf

FILES=\
	SPI_slave.v\
	top.v\
	quadinx.v\
	pll_25_100.v\
	pll_25_40.v\
	signal_sync.v

.PHONY: all clean burn

all:
	# if build folder doesn't exist, create it
	mkdir -p $(BUILD)
	# synthesize using Yosys
	yosys -p "synth_ice40 -top top -blif $(BUILD)/$(PROJ).blif" $(FILES)
	# Place and route using arachne
	arachne-pnr -d $(DEVICE) -P $(FOOTPRINT) -o $(BUILD)/$(PROJ).asc -p $(PINS) $(BUILD)/$(PROJ).blif
	# Convert to bitstream using IcePack
	icepack $(BUILD)/$(PROJ).asc $(BUILD)/$(PROJ).bin

timings:
	icetime -tmd hx$(DEVICE)  $(BUILD)/$(PROJ).asc

burn:
	iceprog $(BUILD)/$(PROJ).bin

clean:
	rm -f build/*

world: clean all
