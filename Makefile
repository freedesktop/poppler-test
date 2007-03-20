CFLAGS = $(shell pkg-config --cflags poppler-glib pango gdk-2.0) -g -Wall -O2
LDLIBS = $(shell pkg-config --libs poppler-glib) -lssl -lpng
PDFNAMES = mask.pdf text.pdf image.pdf type3.pdf cropbox.pdf \
           inline-image.pdf degenerate-path.pdf mask-seams.pdf \
           zero-width.pdf

PDFS = $(addprefix tests/, $(PDFNAMES))

ifdef POPPLER_DIR
	POPPLER_DEPS = $(POPPLER_DIR)/poppler/libpoppler.la
endif

all : test-poppler update-cache $(PDFS)

test-poppler: buffer-diff.o  read-png.o  test-poppler.o  write-png.o  util.o read-cache.o $(POPPLER_DEPS)
ifdef POPPLER_DIR
	$(POPPLER_DIR)/libtool --mode=link gcc -Wall -g -O2  -lpng -lssl -o $@  $^ $(POPPLER_DIR)/poppler/libpoppler.la $(POPPLER_DIR)/glib/libpoppler-glib.la 
endif

update-cache: update-cache.o read-cache.o read-png.o util.o
	$(CC) -Wall $^ -o $@ -lpng -lssl

clean:
	rm -f test-poppler *.o tests/*out.png

# don't build the pdfs by default because some people might not have the right perl setup
$(PDFS): tests/%.pdf: test-gen/%.pl
	cd test-gen && ./$(notdir $<) ../$@
