TARGET=platform# platform or web

CFLAGS = -Wall -pedantic -std=c99 -pthread
LDFLAGS = -pthread

ifeq (platform, $(TARGET))
	CC = gcc
	EXE = micg

	LDFLAGS += -lSDL2
else ifeq (web, $(TARGET))
	CC = emcc
	EXE = micg.html

	CFLAGS += -s USE_SDL=2 -s USE_PTHREADS=1
	LDFLAGS += -s USE_SDL=2 -s USE_PTHREADS=1 --proxy-to-worker # -flto
else
	CC = false
	EXE = micg
endif

SRCFILES = main.c world.c physics.c

OBJFILES = $(addprefix build/, $(patsubst %.c, %.o, $(SRCFILES)))

$(EXE): $(OBJFILES)
	$(CC) $^ -o $@ $(LDFLAGS)

src/main.c: src/main.h src/physics.h
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

