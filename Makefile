CFLAGS = -Wall

bin/vme2fd : src/vme2fd.c
	mkdir -p ./bin
	gcc $(CFLAGS) -o $@ $<

test : bin/vme2fd
	./scripts/vme2fd_test.sh
