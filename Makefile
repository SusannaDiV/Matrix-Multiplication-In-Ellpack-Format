all:
	gcc main.c functionality/multiplication.c functionality/ellpack_utility.c functionality/parser.c -o main -O3
debug:
	gcc -Wall -Wextra main.c functionality/multiplication.c functionality/ellpack_utility.c functionality/parser.c -o main -pedantic -g -fsanitize=address -fsanitize=leak -fsanitize=undefined -Wpedantic
profile:
	gcc main.c functionality/multiplication.c functionality/ellpack_utility.c functionality/parser.c -o main -O3 -g
