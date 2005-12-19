CFLAGS = $(shell pkg-config --cflags poppler-glib pango gdk-2.0) -g
LDLIBS = $(shell pkg-config --libs poppler-glib)

PDFS = $(addprefix tests/, mask.pdf text.pdf image.pdf type3.pdf cropbox.pdf inline-image.pdf)
POPPLER_DIR = ../poppler-build

all : test-poppler $(PDFS)

test-poppler: buffer-diff.o  read-png.o  test-poppler.o  write-png.o  xmalloc.o $(POPPLER_DIR)/poppler/libpoppler.la
	$(POPPLER_DIR)/libtool --mode=link gcc  -g -O2   -o $@  $^ $(POPPLER_DIR)/poppler/libpoppler.la $(POPPLER_DIR)/glib/libpoppler-glib.la 
clean :
	rm test-poppler *.o tests/*out.png

# don't build the pdfs by default because some people might not have the right perl setup
$(PDFS): tests/%.pdf: test-gen/%.pl
	cd test-gen && ./$(notdir $<) ../$@
