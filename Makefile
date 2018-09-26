CFLAGS += -std=c11 -g -Wall -Wextra -ftrapv -Wshadow -Wundef -Wcast-align -Wunreachable-code -O1

SRC = src/*.c
TEST_SRC = test/*.c

OBJ = mem_pool.o
LIB = libmem_pool.so

INCLUDE_DIR = /usr/local/include/mem_pool


.PHONY: test


build:
	rm -f ./*.o
	$(CC) $(CFLAGS) -c -fpic $(SRC)
	$(CC) -shared -fpic -o $(LIB) ./*.o -lpthread

install: build
	sudo mv $(LIB) /usr/local/lib
	sudo mkdir -p $(INCLUDE_DIR)
	sudo cp include/*.h $(INCLUDE_DIR)

#-D _POSIX_C_SOURCE=199309 Needed by minunit.h to enable some time related stuff
compile-test: install
	$(CC) $(CFLAGS) -D _POSIX_C_SOURCE=199309L $(TEST_SRC) -lmem_pool -o test.o

test: compile-test
	./test.o

test-valgrind: compile-test
	valgrind --track-origins=yes --leak-check=full --show-reachable=yes ./test.o


