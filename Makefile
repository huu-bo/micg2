TARGET=platform# platform or web

CFLAGS = -Wall -pedantic -std=c99 -DGIT_VERSION='"$(GIT_VERSION)"'
LDFLAGS = -lm

GIT_VERSION = $(shell git describe --dirty --always --tags)
BUILD_MARKER = build/$(GIT_VERSION)-$(TARGET).build
DEP = build/
DEPFLAGS = -MT $@ -MMD -MP -MF $(DEP)/$*.d

ifeq (platform, $(TARGET))
	CC = gcc
	EXE ?= micg

	CFLAGS += -ggdb
	LDFLAGS += -lSDL2 -ggdb
else ifeq (web, $(TARGET))
	CC = emcc
	EXE ?= micg.html

	CFLAGS += -s USE_SDL=2 # -s USE_PTHREADS=1
	LDFLAGS += -s USE_SDL=2 --preload-file mod # -s USE_PTHREADS=1 --proxy-to-worker
else
	CC ?= false
	EXE ?= micg
endif

SRCFILES = main.c world.c block.c noise.c texture.c

OBJFILES = $(addprefix build/, $(patsubst %.c, %.o, $(SRCFILES)))

$(EXE): $(OBJFILES)
	$(CC) $^ -o $@ $(LDFLAGS)

build/%.o: src/%.c Makefile $(BUILD_MARKER) | build
	$(CC) $(DEPFLAGS) $(CFLAGS) -c $< -o $@

build:
	mkdir build

$(BUILD_MARKER): | build
	rm -fv build/*.build
	touch $(BUILD_MARKER)

.PHONY: full
full: clean micg

.PHONY: clean
clean:
	rm -fv $(OBJFILES)
	rm -frv build
	rm -fv $(EXE) micg.*

DEPFILES := $(OBJFILES:%.o=%.d)
include $(wildcard $(DEPFILES))

