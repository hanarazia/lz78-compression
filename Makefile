CC = clang
CFLAGS = -Wall -Wextra -Werror -Wpedantic $(GMP_CFLAGS)
%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

all: encode decode

encode: encode.o trie.o io.o
	$(CC) $(CFLAGS) encode.o trie.o io.o -o encode -lm

decode: decode.o word.o trie.o io.o
	$(CC) $(CFLAGS) decode.o word.o trie.o io.o -o decode -lm

clean:
	rm -f *.o encode decode

format:
	clang-format -i -style=file *.[ch]
