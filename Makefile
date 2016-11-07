compile-test:
	gcc -g -Wall -Wextra -D MEMORY_POOL_DEBUG src/*.c test/*.c -o test.o

test:
	make compile-test
	./test.o

test-valgrind:
	make compile-test
	valgrind --track-origins=yes --leak-check=full --show-reachable=yes ./test.o


.PHONY: test
