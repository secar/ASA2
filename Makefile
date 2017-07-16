BIN = omp
SRC = omp.c
CFLAGS = -Wfatal-errors -fdiagnostics-color=always -ftrapv -Wall -Wextra -Werror --std=c99 

omp:
	cc -O3 $(CFLAGS) -o $(BIN) $(SRC)
debug:
	cc -g3 $(CFLAGS) -o $(BIN) $(SRC)
test: debug
	./test
