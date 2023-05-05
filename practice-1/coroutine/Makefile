all: 
	gcc -o main main.c coroutine.c utils.c -fstack-protector-all -pthread -O0
run:
	./main

clean:
	rm main

test:
	gcc -o test test.c utils.c -g

p:
	gcc -o ptest test.c coroutine.c utils.c -g -pthread -O0
c:
	rm ptest