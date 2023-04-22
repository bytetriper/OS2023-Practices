c: 
	gcc -o main main.c coroutine.c utils.c -fstack-protector-all
run:
	./main

clean:
	rm main

test:
	gcc -o test test.c utils.c -g

ptest:
	gcc -o ptest test.c coroutine.c utils.c -g -pthread