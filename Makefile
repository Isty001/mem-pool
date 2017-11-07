CFLAGS += -std=c11 -g -Wall -Wextra -ftrapv -Wshadow -Wundef -Wcast-align -Wunreachable-code

SRC = src/*.c
TEST_SRC = test/*.c

OBJ = mem_pool.o
LIB = libmem_pool.so

INCLUDE_DIR = /usr/local/include/mem_pool


.PHONY: test


build:
	rm -f ./*.o
	$(CC) $(CFLAGS) -lpthread -c -fpic $(SRC)
	$(CC) -shared -fpic -o $(LIB) ./*.o

install: build
	sudo mv $(LIB) /usr/local/lib
	sudo mkdir -p $(INCLUDE_DIR)
	sudo cp include/*.h $(INCLUDE_DIR)

compile-test: install
	#				needed by minunit.h to enable time stuff
	gcc $(CFLAGS) -D _POSIX_C_SOURCE=199309L $(TEST_SRC) -lmem_pool -o test.o

test: compile-test
	./test.o

test-valgrind: compile-test
	valgrind --track-origins=yes --leak-check=full --show-reachable=yes ./test.o


