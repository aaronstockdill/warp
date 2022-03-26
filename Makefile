CC=clang
OPTS=-O3

warptool: bin/warptool

bin/warptool: src/warptool.c bin
	$(CC) $(OPTS) -o $@ $<

bin:
	mkdir bin
