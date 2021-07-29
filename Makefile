#CC := em++
CC := gcc

SRCDIR := src
BUNDLEDIR := rhythm-experiments
INCDIR := include
ASSETDIR := assets
TARGETDIR := target
TARGET := $(TARGETDIR)/index.html

VERSION := 1.2.0

$(TARGET): $(SRCDIR)/main.c
	mkdir -p $(TARGETDIR)
	#$(CC) $^ -s USE_SDL=2 -o $(TARGET) --preload-file $(ASSETDIR) --shell-file assets/shell_minimal.html -O3
	$(CC) $^ -o ${TARGETDIR}/game -lSDL2 -I${INCDIR} $(shell sdl2-config --cflags --libs)  -O3

serve:
	python2 -m SimpleHTTPServer

clean:
	rm -r $(TARGET)

bundle:
	mkdir -p $(BUNDLEDIR)
	rm -r $(BUNDLEDIR)/*
	cp $(TARGETDIR)/* LICENSE -t $(BUNDLEDIR)
	zip -r $(BUNDLEDIR)-$(VERSION).zip $(BUNDLEDIR)
