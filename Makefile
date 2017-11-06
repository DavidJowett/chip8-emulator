OBJECTS=chip8.o
MOBJECTS=main.o
TOBJECTS=chip8_test.o

all: ${OBJECTS} ${MOBJECTS}
	gcc -o chip8 ${OBJECTS} ${MOBJECTS}
test: ${OBJECTS} ${TOBJECTS}
	gcc -o test ${OBJECTS} ${TOBJECTS} -lcheck -lpthread

%.o: %.c
	gcc -c $< -o $@

clean:
	rm -f test chip8 ${OBJECTS} ${MOBJECTS} ${TOBJECTS}
