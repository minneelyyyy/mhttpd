
OBJS = src/main.o src/mhttpd.o src/http.o src/keyval.o src/strntok.o

all: mhttpd

CFLAGS += -D_XOPEN_SOURCE=700

.PHONY: all clean
.SUFFIXES: .c .o

.c.o:
	$(CC) $(CFLAGS) -c -o $@ $<

mhttpd: $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS)

clean:
	rm -f mhttpd $(OBJS)
