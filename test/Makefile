CC=g++
RM=rm -rf
CFLAGS=-Wall -O2

PROGRAM=sensor_test
OBJS=sensor_test.o
OBJS+=sensor_wacoh_udynpick.o

.SUFFIXES: .c .o

$(PROGRAM):$(OBJS)
		$(CC) -o $(PROGRAM) $^

.c.o:
		$(CC) $(CFLAGS) -c $<

.PHONY: clean

clean:
	$(RM) $(PROGRAM) $(OBJS)
	$(RM) $(PROGRAM) $(OBJS)

check:
	cppcheck --enable=all .
