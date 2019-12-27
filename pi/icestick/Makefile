CFLAGS+=-Wall -Werror

OBJ=\
	dump.o

PROGS=dump

all: $(PROGS)

tank: $(OBJ)
	$(LINK.c) -o $@ $^

clean:
	rm -f $(OBJ) $(PROGS)

world: clean all
