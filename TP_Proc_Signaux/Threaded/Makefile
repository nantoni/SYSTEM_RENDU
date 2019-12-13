SRC = $(wildcard ./*.c)
LDFLAGS = -pthread
OBJ = $(notdir $(patsubst %.c,%.o,$(SRC)))

all: main

main: $(OBJ)
	gcc -g -o $@ $^ $(LDFLAGS) 

%.o: %.c %.h
	gcc -o $@ -c $<

clean:
	rm *.o

.PHONY: clean
