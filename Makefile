TARGET=platform# platform or web

CFLAGS = -Wall -pedantic -std=c99
LDFLAGS = -lm

ifeq (platform, $(TARGET))
	CC = gcc
	EXE = micg

	CFLAGS += -ggdb
	LDFLAGS += -lSDL2 -ggdb
else ifeq (web, $(TARGET))
	CC = emcc
	EXE = micg.html

	CFLAGS += -s USE_SDL=2 # -s USE_PTHREADS=1
	LDFLAGS += -s USE_SDL=2 # -s USE_PTHREADS=1 --proxy-to-worker
else
	CC = false
	EXE = micg
endif

SRCFILES = main.c world.c block.c noise.c

OBJFILES = $(addprefix build/, $(patsubst %.c, %.o, $(SRCFILES)))

$(EXE): $(OBJFILES)
	$(CC) $^ -o $@ $(LDFLAGS)

src/main.c: src/main.h src/physics.h src/world.h
src/world.c: src/world.h src/main.h src/block.h src/noise.h
src/block.c: src/block.h

build/%.o: src/%.c src/test.h Makefile | build
	$(CC) $(CFLAGS) -c $< -o $@

build:
	mkdir build

.PHONY: full
full: clean micg

.PHONY: clean
clean:
	rm -fv $(OBJFILES)
	rm -frv build
	rm -fv $(EXE) $(patsubst %.html, %.wasm, $(EXE)) $(patsubst %.html, %.js, $(EXE))

