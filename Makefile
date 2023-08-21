CC = gcc
EXE = micg

CFLAGS = -Wall -pedantic -std=c99
LDFLAGS = -lSDL2
SRCFILES = main.c world.c

OBJFILES = $(addprefix build/, $(patsubst %.c, %.o, $(SRCFILES)))

$(EXE): $(OBJFILES)
	$(CC) $^ -o $@ $(LDFLAGS)

src/main.c: src/main.h
src/main.h: src/world.h
src/world.c: src/world.h src/main.h src/block.h

build/%.o: src/%.c src/test.h Makefile | build
	$(CC) $(CFLAGS) -c $< -o $@

build:
	mkdir build

.PHONY: clean
clean:
	rm -fv $(OBJFILES)
	rm -frv build
	rm -fv $(EXE) $(patsubst %.html, %.wasm, $(EXE)) $(patsubst %.html, %.js, $(EXE))

