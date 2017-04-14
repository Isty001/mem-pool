compile-test:
	gcc -g -Wall -Wextra src/*.c test/*.c -lpthread -o test.o

test:
	make compile-test
	./test.o

test-valgrind:
	make compile-test
	valgrind --track-origins=yes --leak-check=full --show-reachable=yes ./test.o


.PHONY: test
