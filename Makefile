CFLAGS = $(shell pkg-config --cflags poppler-glib)
LDLIBS = $(shell pkg-config --libs poppler-glib)

all : test-poppler

test-poppler: buffer-diff.o  read-png.o  test-poppler.o  write-png.o  xmalloc.o

clean :
	rm test-poppler *.o tests/*out.png
