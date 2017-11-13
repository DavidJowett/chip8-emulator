OBJECTS=src/chip8.o src/runtime_error.o src/ui.o src/shader.o
MOBJECTS=src/main.o
TOBJECTS=test/chip8_test.o test/runtime_error_test.o test/main.o
CFLAGS=-g -Wall -Wno-implicit-function-declaration
LFLAGS=-lcheck -lpthread `pkg-config --libs glew glfw3`

ifeq ($(coverage), true)
	CFLAGS+=-fprofile-arcs -ftest-coverage
	LFLAGS+=-lgcov
endif

all: ${OBJECTS} ${MOBJECTS}
	gcc -o chip8 ${OBJECTS} ${MOBJECTS} ${LFLAGS}
test: ${OBJECTS} ${TOBJECTS}
	gcc -o tests ${OBJECTS} ${TOBJECTS} ${LFLAGS}

%.o: %.c
	gcc ${CFLAGS} -c $< -o $@

clean:
	rm -f test chip8 ${OBJECTS} ${MOBJECTS} ${TOBJECTS}
	rm -f *.gcno *.gcda coverage.info
	rm -rf coverage
report:
	lcov --capture --directory . --output-file coverage.info
	genhtml coverage.info --output-directory coverage
