CC = gcc
CFLAGS = -g
TARGETS = master slave
OBJS = master.o slave.o

all: $(OBJS) $(TARGETS)
master: master.o
slave: slave.o
%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)
clean:
	rm -f $(OBJS) $(TARGETS) *.log logfile logfile.* cstest