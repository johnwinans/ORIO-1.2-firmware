CFLAGS+=-Wall -Werror
LDLIBS+=-lz

OBJ=\
	tank.o

all: tank

tank: $(OBJ)
	$(LINK.c) -o $@ $^ $(LDLIBS)

clean:
	rm -f $(OBJ) tank

world: clean all
