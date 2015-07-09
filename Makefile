PROG = m3d-linux
CC = g++
SRCS = main.cpp gcode.cpp printer.cpp
CFLAGS = -Wall -std=c++14 -O3

all: $(PROG)

$(PROG):   $(SRCS)
	$(CC) $(CFLAGS) -o $(PROG) $(SRCS) $(LIBS)

clean:
	rm -f $(PROG)

run:
	./$(PROG)

install:
	install $(PROG) /usr/sbin/$(PROG)

package:
	cp $(PROG) $(PROG)-0.10
	cd $(PROG)-0.10 && fakeroot dpkg-buildpackage -b
