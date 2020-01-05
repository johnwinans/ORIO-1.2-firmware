# Project setup
PROJ      = blinky
BUILD     = ./build

# for 1K devices
DEVICE    = 1k
FOOTPRINT = tq144
PINS=pinmap-icestick.pcf

# for 4K devices
#DEVICE    = 8k
#FOOTPRINT = tq144:4k
#PINS=pinmap-2057.pcf

FILES=\
	SPI_slave.v\
	mojo_top.v\
	quadin.v\
	signal_sync.v

.PHONY: all clean burn

all:
	# if build folder doesn't exist, create it
	mkdir -p $(BUILD)
	# synthesize using Yosys
	yosys -p "synth_ice40 -top mojo_top -blif $(BUILD)/$(PROJ).blif" $(FILES)
	# Place and route using arachne
	arachne-pnr -d $(DEVICE) -P $(FOOTPRINT) -o $(BUILD)/$(PROJ).asc -p $(PINS) $(BUILD)/$(PROJ).blif
	# Convert to bitstream using IcePack
	icepack $(BUILD)/$(PROJ).asc $(BUILD)/$(PROJ).bin

timings:
	icetime -tmd hx1k  $(BUILD)/$(PROJ).asc

burn:
	iceprog $(BUILD)/$(PROJ).bin

clean:
	rm -f build/* log

world: clean all
