CC := gcc

SRCDIR := src
BUNDLEDIR := rhythm-experiments
INCDIR := include
ASSETDIR := assets
TARGETDIR := target
BUILDDIR := $(TARGETDIR)/obj
TARGET := $(TARGETDIR)/game

SDLFLAGS := $(shell sdl2-config --cflags --libs)

LFLAGS := -lSDL2_mixer
CFLAGS := -I $(INCDIR) $(SDLFLAGS) $(LFLAGS)

VERSION := 0.1.0

.PHONY: all

all: \
		$(BUILDDIR)/main.o \
		$(BUILDDIR)/level.o \
		$(BUILDDIR)/graphics.o
	mkdir -p $(TARGETDIR)
	$(CC) $^ -o $(TARGET) $(CFLAGS)

$(BUILDDIR)/%.o: $(SRCDIR)/%.c
	mkdir -p $(BUILDDIR)
	$(CC) $^ -c -o $@ $(CFLAGS)

clean:
	rm -r $(TARGET)
	rm -r $(BUILDDIR)

bundle:
	mkdir -p $(BUNDLEDIR)
	rm -r $(BUNDLEDIR)/*
	cp $(TARGETDIR)/* LICENSE -t $(BUNDLEDIR)
	zip -r $(BUNDLEDIR)-$(VERSION).zip $(BUNDLEDIR)
