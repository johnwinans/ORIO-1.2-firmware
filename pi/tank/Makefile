CFLAGS+=-Wall -Werror
LDLIBS+=-lz

OBJ=\
	tank.o\
	crcccitt.o

all: tank

tank: $(OBJ)
	$(LINK.c) -o $@ $^

clean:
	rm -f $(OBJ) tank spin

world: clean all
