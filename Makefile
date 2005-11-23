CFLAGS = $(shell pkg-config --cflags poppler-glib pango gdk-2.0)
LDLIBS = $(shell pkg-config --libs poppler-glib)

PDFS = $(addprefix tests/, mask.pdf text.pdf image.pdf type3.pdf)

all : test-poppler $(PDFS)

test-poppler: buffer-diff.o  read-png.o  test-poppler.o  write-png.o  xmalloc.o

clean :
	rm test-poppler *.o tests/*out.png

# don't build the pdfs by default because some people might not have the right perl setup
$(PDFS): tests/%.pdf: test-gen/%.pl
	cd test-gen && ./$(notdir $<) ../$@
