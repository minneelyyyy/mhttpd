
OBJS = src/main.o

all: mhttpd

.PHONY: all clean
.SUFFIXES: .c .o

.c.o:
	$(CC) $(CFLAGS) -c -o $@ $<

mhttpd: $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS)

clean:
	rm -f mhttpd $(OBJS)
