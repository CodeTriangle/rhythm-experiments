CC := gcc

SRCDIR := src
BUNDLEDIR := rhythm-experiments
INCDIR := include
ASSETDIR := assets
TARGETDIR := target
TARGET := $(TARGETDIR)/index.html

SDLFLAGS := $(shell sdl2-config --cflags --libs)

VERSION := 0.1.0

$(TARGET): $(SRCDIR)/main.c
	mkdir -p $(TARGETDIR)
	$(CC) $^ -o ${TARGETDIR}/game -I${INCDIR} ${SDLFLAGS} -lSDL2_mixer

clean:
	rm -r $(TARGET)

bundle:
	mkdir -p $(BUNDLEDIR)
	rm -r $(BUNDLEDIR)/*
	cp $(TARGETDIR)/* LICENSE -t $(BUNDLEDIR)
	zip -r $(BUNDLEDIR)-$(VERSION).zip $(BUNDLEDIR)
