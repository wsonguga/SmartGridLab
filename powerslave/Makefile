#Makefile for PowerSlave
PROG = POWER_SLAVE
OBJS = main.o
CC = g++
CFLAGS = -lgsl -lgslcblas -lpthread
PREFIX = /usr/local
.SUFFIXES = .cpp .o
.cpp.o:
	$(CC) $(CFLAGS) -c $*.cpp
$(PROG): $(OBJS)
	 $(CC) -o $(PROG) $(OBJS) $(CFLAGS)
clean:
	rm -f $(OBJS)
	rm -f $(PROG)
install:
	rm -f $(PREFIX)/sbin/$(PROG)
	cp $(PROG) $(PREFIX)/sbin/$(PROG)
uninstall:
	rm -f $(PREFIX)/sbin/$(PROG)
