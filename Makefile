CFLAGS = -Wall

bin/vme2fd : src/vme2fd.c
	gcc $(CFLAGS) -o $@ $<
